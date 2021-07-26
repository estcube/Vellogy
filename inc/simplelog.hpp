/**
 * @file simplelog.hpp
 *
 * This file provides the SimpleLog class
 * which is meant to be used in cases where the datapoints logged are not captured periodically
 * and spaced far apart in time (for example, error logging)
 */

#ifndef SIMPLELOG_H
#define SIMPLELOG_H

#include "baselog.hpp"

namespace Logging {

/**
 * Log class for aperiodic data logging (datapoints are spaced far apart and unevenly in time)
 *
 * @tparam T datatype of datapoints held in the log
 */
template <class T>
class SimpleLog : public BaseLog<T> {
    protected:
        /**
         * Write the capturing time of a datapoint to the data queue
         */
        void write_to_queue_timestamp(
            time_t timestamp    ///< [in] Capturing time(stamp) of a datapoint
        ) {
            this->write_to_queue(&timestamp, sizeof(timestamp));
        }

        /**
         * Write a datapoint to the data queue
         */
        void write_to_queue_datapoint(
            T& data     ///< [in] Datapoint to be written to the queue
        ) {
            this->write_to_queue(&data, sizeof(data));
        }

    public:
        /**** Constructors ****/

        /**
         * Initialize the log with the given file (no meta- and indexfile)
         */
        SimpleLog(
            uint8_t* file   ///< [in] Pointer to the file where datapoints will be saved
        )
            : BaseLog<T>(file)
        {
            // If this is a brand new file without previous datapoints, write the mandatory decode info into the beginning of the file
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

        /**
         * Initialize the log (from a metafile) held in the file given
         */
        SimpleLog(
            uint8_t* metafile,  ///< [in] Pointer to the file where metainfo will be saved
            uint8_t* indexfile, ///< [in] Pointer to the file where index entries will be saved
            uint8_t* file       ///< [in] Pointer to the file where datapoints will be saved
        )
            : BaseLog<T>(metafile, indexfile, file)
        {
            // If this is a brand new file without previous datapoints, write the mandatory decode info into the beginning of the file
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

        /**
         * Initialize the log from a log slice
         */
        SimpleLog(
            LogSlice<SimpleLog, T>* slice,      ///< [in] Log slice containing the data to be copied into the new log
            uint8_t* new_file                   ///< [in] Pointer to the file where slice contents will be copied
        )
            : SimpleLog<T>(new_file)
        {
            // Copy slice into the file provided
            std::memcpy(this->file + this->file_size, slice->get_file() + slice->get_start_location(), slice->get_end_location() - slice->get_start_location());
            this->file_size += slice->get_end_location() - slice->get_start_location();
        }

        /**
         * Free allocated buffers on object destruction
         */
        ~SimpleLog() {
            vPortFree(this->data_queue);
            vPortFree(this->double_buffer);
        }

        /**** Main functionality ****/

        /**
         * Log data, attach timestamp in function
         */
        void log(
            T& data     ///< [in] Datapoint to be logged
        );

        /**
         * Log data with a given timestamp in the file
         */
        void log(
            T& data,            ///< [in] Datapoint to be logged
            time_t timestamp    ///< [in] Capturing time of the datapoint
        ) {
            this->write_to_queue_timestamp(timestamp);
            this->write_to_queue_datapoint(data);

            this->switch_buffers();
            this->entries_added++;

            // If enough data has been logged for a new index entry, create it
            if (this->indexfile != NULL && this->entries_added % LOG_SIMPLE_INDEX_DENSITY == 1) {
                this->write_to_index(timestamp, this->file_size - sizeof(time_t) - sizeof(T));
            }

            this->queue_len = 0;
        }

        /**
         * Read an array of log entries from the chosen time period
         */
        LogSlice<SimpleLog,T> slice(
            time_t start_ts,    ///< [in] Starting point of the chosen time period
            time_t end_ts       ///< [in] Endpoint of the chosen time period
        ) {
            return log_slice<SimpleLog,T>(this->file, this->file_size, this->indexfile, this->indexfile_size, start_ts, end_ts);
        }

        /**
         * Read an array of log entries from the chosen time period
         * Write resulting slice into new_file
         */
        LogSlice<SimpleLog,T> slice(
            time_t start_ts,    ///< [in] Starting point of the chosen time period
            time_t end_ts,      ///< [in] Endpoint of the chosen time period
            uint8_t* new_file   ///< [in] File where the log slice is to be written
        ) {
            return log_slice<SimpleLog,T>(this->file, this->file_size, this->indexfile, this->indexfile_size, start_ts, end_ts, new_file);
        }

        /**** Utility functions ****/

        /**
         * Write all datapoints in volatile memory to file
         * Dummy function to make the common Log interface more general
         */
        void flush() {
            return;
        }

        /**
         * Return resolution of log timestamps
         * Dummy function to make the common Log interface more general
         */
        int8_t get_resolution() {
            return -128;
        }

        /**
         * Signify period change in incoming data on user level
         * Dummy function to make the common Log interface more general
         */
        void period_change() {
            return;
        }

        /**** Static utility functions ****/

        /**
         * Find closest log entry whose timestamp is less than or equal to given timestamp, starting from address file + search_location
         */
        static uint32_t find_log_entry(
            uint8_t* file,              ///< [in] Pointer to the log file from where we search the entry
            uint32_t file_size,         ///< [in] Size of the log file from where we search the entry
            uint8_t datapoint_size,     ///< [in] Size of datapoints in the log file in bytes
            time_t timestamp,           ///< [in] Timestamp to be searched
            uint32_t search_location,   ///< [in] From which byte in the file does the search start
            bool succeeding             ///< [in] Do we want the entry containing the timestamp or the entry after it
        ) {
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

        /**
         * Find first logged timestamp in the log file
         */
        static time_t find_first_timestamp(
            uint8_t* file,          ///< [in] Pointer to the log file
            uint32_t file_size,     ///< [in] Size of the log file
            uint32_t file_break     ///< [in] Break in the log file (separating last logged datapoints from first logged ones)
        ) {
            time_t first_ts;
            std::memcpy(&first_ts, file + LOG_MANDATORY_DECODE_INFO_SIZE, sizeof(time_t));
            return first_ts;
        }

        /**
         * Find last logged timestamp in the log file
         */
        static time_t find_last_timestamp(
            uint8_t* file,          ///< [in] Pointer to the log file
            uint32_t file_break,    ///< [in] Size of the log file
            int8_t resolution       ///< [in] Resolution of timestamps in the log file
        ) {
            time_t last_ts;
            std::memcpy(&last_ts, file + file_break - sizeof(T) - sizeof(time_t), sizeof(time_t));
            return last_ts;
        }
};

}

#endif
