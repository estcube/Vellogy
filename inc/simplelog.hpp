#ifndef SIMPLELOG_H
#define SIMPLELOG_H

#include "baselog.hpp"

template <class T>
class SimpleLog : public BaseLog<T> {
    public:
        // Initialize the log with the given file (no meta- and indexfile)
        SimpleLog(uint8_t* file)
            : BaseLog<T>(file)
        {
            if (!this->file_size) {
                log_file_type_t file_type = LOG_SIMPLE;
                log_data_type_t data_type = LOG_INT32_T; // TODO: non-dummy value

                std::memcpy(this->file + this->file_size, &file_type, sizeof(log_file_type_t));
                this->file_size += sizeof(log_file_type_t);
                std::memcpy(this->file + this->file_size, &data_type, sizeof(log_data_type_t));
                this->file_size += sizeof(log_data_type_t);
            }

            this->data_queue = (uint8_t *)pvPortMalloc(sizeof(time_t) + sizeof(T));
            this->double_buffer = (uint8_t *)pvPortMalloc(sizeof(time_t) + sizeof(T));
        }

        // Initialize the log (from a metafile) held in file pointed to by the third argument
        SimpleLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file)
            : BaseLog<T>(metafile, indexfile, file)
        {
            if (!this->file_size) {
                log_file_type_t file_type = LOG_SIMPLE;
                log_data_type_t data_type = LOG_INT32_T; // TODO: non-dummy value

                std::memcpy(this->file + this->file_size, &file_type, sizeof(log_file_type_t));
                this->file_size += sizeof(log_file_type_t);
                std::memcpy(this->file + this->file_size, &data_type, sizeof(log_data_type_t));
                this->file_size += sizeof(log_data_type_t);
            }

            this->data_queue = (uint8_t *)pvPortMalloc(sizeof(time_t) + sizeof(T));
            this->double_buffer = (uint8_t *)pvPortMalloc(sizeof(time_t) + sizeof(T));
        }

        // Initialize the log from a log slice
        SimpleLog(LogSlice<SimpleLog, T>* slice, uint8_t* new_file)
            : SimpleLog<T>(new_file)
        {
            // Copy slice into the file provided
            std::memcpy(this->file + this->file_size, slice->get_file() + slice->get_start_location(), slice->get_end_location() - slice->get_start_location());
            this->file_size += slice->get_end_location() - slice->get_start_location();
        }

        // Free allocated buffers on object destruction
        ~SimpleLog() {
            vPortFree(this->data_queue);
            vPortFree(this->double_buffer);
        }

        // Log data (implemented differently for different types), attach timestamp in function
        void log(T& data);

        void log(T& data, time_t timestamp) {
            this->write_to_queue(&timestamp, sizeof(time_t));
            this->write_to_queue(&data, sizeof(T));

            this->switch_buffers();
            this->entries_added++;

            // If enough data has been logged for a new index entry, create it
            if (this->indexfile != NULL && this->entries_added % INDEX_DENSITY == 1) {
                this->write_to_index(timestamp, this->file_size - sizeof(time_t) - sizeof(T));
            }

            this->queue_len = 0;
        }

        // Dummy function to make the common Log interface more general
        int8_t get_resolution() {
            return -128;
        }

        // Dummy function to make the common Log interface more general
        void flush() {
            return;
        }

        // Read an array of log entries from the chosen time period
        LogSlice<SimpleLog,T> slice(time_t start_ts, time_t end_ts) {
            return log_slice<SimpleLog,T>(this->file, this->file_size, this->indexfile, this->indexfile_size, start_ts, end_ts, -128);
        }

        // Find closest log entry whose timestamp is less than or equal to given timestamp, starting from address file + search_location
        static uint32_t find_log_entry(uint8_t* file, uint8_t datapoint_size, time_t timestamp, uint32_t search_location, bool succeeding) {
            uint32_t reading_location = search_location;

            bool done = false;
            while (!done) {
                reading_location -= sizeof(time_t) + datapoint_size;
                time_t entry_ts;
                std::memcpy(&entry_ts, file + reading_location, sizeof(time_t));

                // The timestamp we are looking for is in this entry
                if (timestamp >= entry_ts) {
                    // if we need to find the succeeding entry to the one containing the timestamp instead
                    if (succeeding) reading_location += sizeof(time_t) + datapoint_size;
                    // Exit search loop
                    done = true;
                }
            }

            return reading_location;
        }

        // Find last logged timestamp in the log file
        static time_t find_last_timestamp(uint8_t* file, uint32_t file_size, int8_t resolution) {
            time_t last_ts;
            std::memcpy(&last_ts, file + file_size - sizeof(T) - sizeof(time_t), sizeof(time_t));
            return last_ts;
        }
};

#endif
