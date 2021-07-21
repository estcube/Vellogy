#ifndef PERIODICLOG_H
#define PERIODICLOG_H

#include "baselog.hpp"

#define DATAPOINTS_IN_ENTRY 20

template <class T>
class PeriodicLog : public BaseLog<T> {
    private:
        time_t entry_timestamp;
        time_t last_timestamp;
        uint32_t data_added; // How many datapoints have been added to the queue under the last entry timestamp

        constexpr uint32_t min_queue_size() {
            return sizeof(time_t) * 2 + DATAPOINTS_IN_ENTRY * sizeof(T) + sizeof(uint32_t);
        }

        void write_to_queue_timestamp() {
            this->write_to_queue(&(this->last_timestamp), sizeof(this->last_timestamp));
        }

        void write_to_queue_datapoint(T& data) {
            this->write_to_queue(&data, sizeof(data));
        }

        void write_to_queue_data_added() {
            this->write_to_queue(&(this->data_added), sizeof(this->data_added));
            this->switch_buffers();
        }

        void end_entry() {
            this->write_to_queue_timestamp();
            this->write_to_queue_data_added();

            // A new entry was written to the log file
            this->entries_added++;

            // Create an index entry (write it to indexfile) if enough log entries have been added to the log file
            if (this->indexfile != NULL && this->entries_added % INDEX_DENSITY == 1) {
                this->write_to_index(this->entry_timestamp, this->file_size - this->queue_len);
            }

            // Reset log state
            this->entry_timestamp = 0;
            this->last_timestamp = 0;
            this->data_added = 0;

            // Reset queue state
            this->queue_len = 0;
        }

    public:
        /**** Constructors ****/

        // Initialize the log with the given file (no meta- and indexfile)
        PeriodicLog(uint8_t* file)
            : BaseLog<T>(file)
            , entry_timestamp{0}
            , last_timestamp{0}
            , data_added{0}
        {
            if (!this->file_size) {
                log_file_type_t file_type = LOG_PERIODIC;
                log_data_type_t data_type = LOG_INT32_T; // TODO: non-dummy value

                std::memcpy(this->file + this->file_size, &file_type, sizeof(log_file_type_t));
                this->file_size += sizeof(log_file_type_t);
                // TODO: non-dummy value
                std::memcpy(this->file + this->file_size, &data_type, sizeof(log_data_type_t));
                this->file_size += sizeof(log_data_type_t);
            }

            this->data_queue = (uint8_t *)pvPortMalloc(this->min_queue_size());
            this->double_buffer = (uint8_t *)pvPortMalloc(this->min_queue_size());
        }

        // Initialize the log (from a metafile) held in file pointed to by the third argument
        PeriodicLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file)
            : BaseLog<T>(metafile, indexfile, file)
            , entry_timestamp{0}
            , last_timestamp{0}
            , data_added{0}
        {
            if (!this->file_size) {
                log_file_type_t file_type = LOG_PERIODIC;
                log_data_type_t data_type = LOG_INT32_T; // TODO: non-dummy value

                std::memcpy(this->file + this->file_size, &file_type, sizeof(log_file_type_t));
                this->file_size += sizeof(log_file_type_t);
                // TODO: non-dummy value
                std::memcpy(this->file + this->file_size, &data_type, sizeof(log_data_type_t));
                this->file_size += sizeof(log_data_type_t);
            }

            this->data_queue = (uint8_t *)pvPortMalloc(this->min_queue_size());
            this->double_buffer = (uint8_t *)pvPortMalloc(this->min_queue_size());
        }

        // Initialize the log from a log slice
        PeriodicLog(LogSlice<PeriodicLog, T>* slice, uint8_t* new_file)
            : PeriodicLog<T>(new_file)
        {
            // Copy slice into the file provided
            std::memcpy(this->file + this->file_size, slice->get_file() + slice->get_start_location(), slice->get_end_location() - slice->get_start_location());
            this->file_size += slice->get_end_location() - slice->get_start_location();
        }

        // Free allocated buffers on object destruction
        ~PeriodicLog() {
            vPortFree(this->data_queue);
            vPortFree(this->double_buffer);
        }

        /**** Main functionality ****/

        // Log data (implemented differently for different types), attach timestamp in function
        void log(T& data);

        // Log data with a given timestamp in the file
        void log(T& data, time_t timestamp) {
            this->last_timestamp = timestamp;

            // Start logging under a new timestamp
            if (!this->entry_timestamp) {
                this->entry_timestamp = timestamp;
                this->write_to_queue_timestamp();
            }

            this->write_to_queue_datapoint(data);
            this->data_added++;

            // If entry is full, write it to memory
            if (this->data_added >= DATAPOINTS_IN_ENTRY) {
                this->end_entry();
            }
        }

        // Read an array of log entries from the chosen time period
        LogSlice<PeriodicLog,T> slice(time_t start_ts, time_t end_ts) {
            return log_slice<PeriodicLog,T>(this->file, this->file_size, this->indexfile, this->indexfile_size, start_ts, end_ts, -128);
        }

        /**** Utility functions ****/

        // Write all datapoints in volatile memory to file
        void flush() {
            this->end_entry();
        }

        // Return resolution of log timestamps
        int8_t get_resolution() {
            return -128;
        }

        // Signify period change in incoming data on user level
        void period_change() {
            this->end_entry();
        }

        /**** Static utility functions ****/

        // Find closest log entry whose timestamp is less than or equal to given timestamp, starting from address file + search_location
        static uint32_t find_log_entry(uint8_t* file, uint8_t datapoint_size, time_t timestamp, uint32_t search_location, bool succeeding) {
            uint32_t reading_location = search_location;

            bool done = false;
            while (!done) {
                reading_location -= sizeof(uint32_t);
                uint32_t data_added;
                std::memcpy(&data_added, file + reading_location, sizeof(uint32_t));

                // Let's jump ahead and read the timestamp in the beginning of the entry
                uint32_t data_size = data_added * datapoint_size;
                reading_location -= data_size + 2 * sizeof(time_t);
                time_t entry_ts;
                std::memcpy(&entry_ts, file + reading_location, sizeof(time_t));

                // The timestamp we are looking for is in this entry
                if (timestamp >= entry_ts) {
                    // if we need to find the succeeding entry to the one containing the timestamp instead
                    if (succeeding) reading_location += 2 * sizeof(time_t) + data_size + sizeof(uint32_t);
                    // Exit search loop
                    done = true;
                }
            }

            return reading_location;
        }

        // Find last logged timestamp in the log file
        static time_t find_last_timestamp(uint8_t* file, uint32_t file_size, int8_t resolution) {
            time_t last_ts;
            std::memcpy(&last_ts, file + file_size - sizeof(uint32_t) - sizeof(time_t), sizeof(time_t));
            return last_ts;
        }
};

#endif
