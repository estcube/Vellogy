#ifndef REGULARLOG_H
#define REGULARLOG_H

#include "baselog.hpp"

template <class T>
class RegularLog : public BaseLog<T> {
    protected:
        // Logic
        time_t entry_timestamp; // Timestamp in the beginning of the last log entry
        uint8_t data_added; // How many datapoints have been added to the queue under the last timestamp
        const int8_t resolution; // TS_BASE^resolution is the smallest unit of time that needs to separable

        // Calculate minimum possible size in bytes for the data queue (maximum size of log in bytes)
        constexpr uint32_t min_queue_size() {
            uint32_t queue_size = 0;

            queue_size += sizeof(time_t); // Size of timestamp
            queue_size += sizeof(uint8_t); // Size of data_added
            queue_size += (1 << (sizeof(uint8_t) * CHAR_BIT)) * (sizeof(T) + sizeof(uint8_t)); // 2^{size_of_uint8_t_in_bits} * size_of_one_datapoint_with_timedelta

            return queue_size;
        }

        // Round a given timedelta to a multiple of TS_BASE^resolution and divide by TS_BASE^resolution
        uint16_t scale_timedelta(uint64_t timedelta) {
            // Relative precision: how much less precise is the timedelta we can log compared to the actual timestamp resolution
            // For example, if TS_RESOLUTION is -4 and this->resolution is -2 and timestamp subseconds are in decimal system,
            // then timedeltas actually saved are 100 times less precise than the timestamp type allows
            uint32_t rel_precision = pow(TS_BASE, abs(this->resolution - TS_RESOLUTION));

            // For example, if timedelta is 7863 and rel_precision is 100, then lower_bound is 7800 and upper_bound is 7900
            uint16_t lower_bound = timedelta / rel_precision * rel_precision;
            uint16_t upper_bound = (timedelta / rel_precision + 1) * rel_precision;

            return (upper_bound - timedelta < timedelta - lower_bound ? upper_bound : lower_bound) / rel_precision;
        }

        // Write current timestamp in byte form to the data queue
        void write_to_queue_timestamp() {
            this->write_to_queue(&(this->entry_timestamp), sizeof(this->entry_timestamp));
        }

        // Write datapoint in byte form to the data queue
        void write_to_queue_datapoint(T& datapoint) {
            this->write_to_queue(&datapoint, sizeof(datapoint));
        }

        // Write timedelta in byte form to the data queue
        void write_to_queue_timedelta(uint8_t timedelta) {
            this->write_to_queue(&timedelta, sizeof(timedelta));
        }

        // Write current data_added in byte form to the data queue
        void write_to_queue_data_added() {
            this->write_to_queue(&(this->data_added), sizeof(this->data_added));
            this->switch_buffers();
        }

        // Manually close the current entry (write entry to file)
        void end_entry() {
            this->write_to_queue_data_added();
            // A new entry was written to the log file
            this->entries_added++;

            // Create an index entry (write it to indexfile) if enough log entries have been added to the log file
            if (this->indexfile != NULL && this->entries_added % INDEX_DENSITY == 1) {
                this->write_to_index(this->entry_timestamp, this->file_size - this->queue_len);
            }

            // Reset log state
            this->entry_timestamp = 0;
            this->data_added = 0;

            // Reset queue state
            this->queue_len = 0;
        }

    public:
        /**** Constructors ****/

        // Initialize the log with the given file (no meta- and indexfile)
        RegularLog(uint8_t* file, int8_t resolution)
            : BaseLog<T>(file)
            , entry_timestamp{0}
            , data_added{0}
            , resolution{resolution}
        {
            if (!this->file_size) {
                log_file_type_t file_type = LOG_REGULAR;
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
        RegularLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file, int8_t resolution)
            : BaseLog<T>(metafile, indexfile, file)
            , entry_timestamp{0}
            , data_added{0}
            , resolution{resolution}
        {
            if (!this->file_size) {
                log_file_type_t file_type = LOG_REGULAR;
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
        RegularLog(LogSlice<RegularLog, T>* slice, uint8_t* new_file)
            : RegularLog<T>(new_file, slice->get_resolution())
        {
            // Copy slice into the file provided
            std::memcpy(this->file + this->file_size, slice->get_file() + slice->get_start_location(), slice->get_end_location() - slice->get_start_location());
            this->file_size += slice->get_end_location() - slice->get_start_location();
        }

        // Free allocated buffers on object destruction
        ~RegularLog() {
            vPortFree(this->data_queue);
            vPortFree(this->double_buffer);
        }

        /**** Main functionality ****/

        // Log data (implemented differently for different types), attach timestamp in function
        void log(T& data);

        // Log data with a given timestamp in the file
        void log(T& data, time_t timestamp) {
            bool new_entry = false;

            if (!this->entry_timestamp
                || this->scale_timedelta(timestamp - this->entry_timestamp) >= (1 << (sizeof(uint8_t) * CHAR_BIT))
                || this->data_added >= (1 << (sizeof(uint8_t) * CHAR_BIT)) - 1) {
                // If one timestamp resolution is full, write to queue how many datapoints were recorded under the previous timestamp
                if (this->entry_timestamp) {
                    this->end_entry();
                }
                // Create a new timestamp and write it to queue
                this->entry_timestamp = timestamp;
                this->write_to_queue_timestamp();
                // This is a new entry with a new timestamp
                new_entry = true;
            }

            // Write datapoint to queue
            this->write_to_queue_datapoint(data);
            // If time_diff was bigger than 8 bits, then a new entry was created above, so this cast is safe
            uint8_t time_diff = (uint8_t) this->scale_timedelta(timestamp - this->entry_timestamp);
            this->write_to_queue_timedelta(time_diff);

            // The maximum number of datapoints under one timestamp is equal to maximum number of different deltas, 2^(size of uint8_t in bits)
            // To save memory, the number of datapoints written to the end of the entry is one less than there actually are
            // For example, 1 datapoint is written as 0, 2 as 1 and so on
            // This works because an entry with only a timestamp and 0 datapoints is impossible
            // We will not increment data_added on the first datapoint (when delta is zero and new_entry is true)
            if (new_entry) {
                this->data_added = 0;
                new_entry = false;
                return;
            }

            this->data_added++;
        }

        // Read an array of log entries from the chosen time period
        LogSlice<RegularLog,T> slice(time_t start_ts, time_t end_ts) {
            return log_slice<RegularLog,T>(this->file, this->file_size, this->indexfile, this->indexfile_size, start_ts, end_ts, this->resolution);
        }

        // Read an array of log entries from the chosen time period
        // Write resulting slice into new_file
        LogSlice<RegularLog,T> slice(time_t start_ts, time_t end_ts, uint8_t* new_file) {
            return log_slice<RegularLog,T>(this->file, this->file_size, this->indexfile, this->indexfile_size, start_ts, end_ts, this->resolution, new_file);
        }

        // Log<T> compress(compression_method_t method); // Compress log with the chosen method
        // Log<T> merge(Log<T> otherLog); // Merge two logs and create a new log

        /**** Utility functions ****/

        // Write all datapoints in volatile memory to file
        void flush() {
            this->end_entry();
        }

        // Return resolution of log timestamps
        int8_t get_resolution() {
            return this->resolution;
        }

        // Dummy function to make the common Log interface more general
        void period_change() {
            return;
        }

        /**** Static utility functions ****/

        // Find closest log entry whose timestamp is less than or equal to given timestamp, starting from address file + search_location
        static uint32_t find_log_entry(uint8_t* file, uint8_t datapoint_size, time_t timestamp, uint32_t search_location, bool succeeding) {
            uint32_t reading_location = search_location;

            bool done = false;
            while (!done) {
                reading_location -= sizeof(uint8_t);
                uint8_t data_added;
                std::memcpy(&data_added, file + reading_location, sizeof(uint8_t));

                // Let's jump ahead and read the timestamp in the beginning of the entry
                uint32_t data_size = (data_added + 1)*(datapoint_size + sizeof(uint8_t));
                reading_location -= data_size + sizeof(time_t);
                time_t entry_ts;
                std::memcpy(&entry_ts, file + reading_location, sizeof(time_t));

                // The timestamp we are looking for is in this entry
                if (timestamp >= entry_ts) {
                    // if we need to find the succeeding entry to the one containing the timestamp instead
                    if (succeeding) reading_location += sizeof(time_t) + data_size + sizeof(uint8_t);
                    // Exit search loop
                    done = true;
                }
            }

            return reading_location;
        }

        // Find last logged timestamp in the log file
        static time_t find_last_timestamp(uint8_t* file, uint32_t file_size, int8_t resolution) {
            uint8_t last_data_added;
            std::memcpy(&last_data_added, file + file_size - sizeof(uint8_t), sizeof(uint8_t));

            uint32_t last_timedelta;
            std::memcpy(&last_timedelta, file + file_size - 2 * sizeof(uint8_t), sizeof(uint8_t));
            // Relative precision of timedelta (see scale_timedelta function for more info)
            uint32_t rel_precision = pow(TS_BASE, abs(resolution - TS_RESOLUTION));
            last_timedelta *= rel_precision;

            time_t last_ts;
            std::memcpy(&last_ts, file + file_size - sizeof(uint8_t) - (last_data_added + 1)*(sizeof(T) + sizeof(uint8_t)) - sizeof(time_t), sizeof(time_t));
            last_ts += last_timedelta;
            return last_ts;
        }
};

#endif
