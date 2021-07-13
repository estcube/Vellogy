#ifndef LOGGING_H
#define LOGGING_H

// C++ includes
#include <cstring>
#include <climits>
#include <ctime>
#include <cstdint>
#include <concepts>
#include <utility>
#include <cmath>

// C includes
#include <FreeRTOS.h>
#include "logging_cfg.h"

/**
 * @file logging.h
 *
 * Module for data logging on ESTCube-2
 * Logging module configuration can be found in @file logging_cfg.h
 *
 * The module allows to:
 * - Create a log in a given file
 * - Save datapoints and their capturing times in the created log file
 * - Search for data logged in a given time interval from the log file
 */

// Smaller stuff:
// TODO: use references
// TODO: kahendotsing templatest välja

// TODO: getter resolution jaoks
// TODO: resolution slicemisel arvesse võtta
// TODO: logifaili algusesse logi tüüp (simple, regular, jne)
// TODO: iga klass eraldi h-faili

// TODO: slice f-nile arg, nö "kuhu mind kirjutatakse" (hiljem)

// NOTE: metafile ülekirjutamine on ok
// NOTE: for future ref: ära salvesta käsitsi failisuurust

// Task list
// 1. Logi decoder refactor + test siinusandmetega
// 1.5. Log object tagastab logi metadata, kus on lisaks decode infole internal state (kui toimub powerout). Logi konstruktor võtab pointeri metafailile.
// 2. Ettevalmistused failisüsteemi tulekuks
// 3. Logi sisukord (iga 10 TS tagant kirje). Kujul timestamp-tema aadress failis(mitmes bait). Optional (kui metafaili ei anta, ei tehta).
// Hoitakse nii Log objekti väljana kui metafailina (peab säilima pärast powerouti).
// 4. CRC iga logi entry kohta. Optional. Otsi CRCde kohta - mis on meile parim? Mathias pakub CRC8. Triple buffer???
// 5. Slicing. Virtuaalsed sliced. Pointer algustimestampile ja pointer lõputimestampile (lähimale järgnevale) failis. Dokumentatsioon!
// Slice peab inheritima koos Log objektiga sama interface'i, kus on compress f-n ja vb midagi veel. Slice-l peab olema võimalus päris logiks saada (kopeerimine).
// 6. SliceLog objektil on compress funktsioon. Loetakse antud aegadega piiratud andmed ja siis compressitakse. Kui kasutaja annab faili, kirjutatakse tulemus faili.
// 7. Kolm logi tüüpi: täiesti random (timestamp + data), somewhat random aga enamasti perioodiline (ts + n * (data + timedelta)), perioodiline (PeriodicLog)

enum compression_method_t {
    LOG_COMPRESSION_FFT,
    LOG_COMPRESSION_HUFFMAN
};

struct log_decode_info_t {
    uint32_t type; // Log semantic data type (what type of sensor, for example)
    uint8_t path[PATH_LEN];
    uint8_t T_formatstring[FORMATSTRING_LEN];
    uint8_t TS_seconds_size; // For timestamp, we only need the number of bytes to deduce the formatstring
    uint8_t TS_subseconds_size;
};

template <class T> class BaseLog;
template <class T> class RegularLog;
template <class T> class SimpleLog;

template<template <class> class T, class E>
concept Loggable = std::is_convertible<T<E>, BaseLog<E>>::value;

template <template <class> class T, class E> class LogSlice;
template<template <class> class T, class E> requires Loggable<T,E> class Log;

template <template <class> class T, class E>
class LogSlice {
    private:
        uint8_t* file;
        uint32_t start_location;
        uint32_t end_location;
        int8_t resolution;

    public:
        LogSlice(uint8_t* file, uint32_t start_location, uint32_t end_location, int8_t resolution)
            : file(file)
            , start_location{start_location}
            , end_location{end_location}
            , resolution{resolution}
        {}

        uint8_t* get_file() {
            return this->file;
        }

        uint32_t get_start_location() {
            return this->start_location;
        }

        uint32_t get_end_location() {
            return this->end_location;
        }

        int8_t get_resolution() {
            return this->resolution;
        }

        Log<T,E> createLog(uint8_t* new_file)  {
            // TODO: allocate mem
            T<E> new_log = T<E>::sliceToLog(new_file, this->file, this->start_location, this->end_location, this->resolution);
            return Log<T,E>(&new_log);
        }
};

template<template <class> class T, class E> requires Loggable<T,E>
class Log {
    private:
        void* obj;
    public:
        Log(T<E>* log_obj) {
            this->obj = log_obj;
        };

        // Get size of log file in bytes
        uint32_t get_file_size() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_file_size();
        }

        // Write current state to metafile
        void save_meta_info() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->save_meta_info();
        }

        // Write all datapoints in volatile memory to file
        void flush() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->flush();
        }

        // Write given datapoint with given timestamp to log file
        void log(E& data, time_t timestamp) {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->log(data, timestamp);
        };

        // Read an array of log entries from the chosen time period
        LogSlice<T,E> slice(time_t start_ts, time_t end_ts) {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->slice(start_ts, end_ts);
        }
};

template <class T>
class BaseLog {
    protected:
        // Files
        log_decode_info_t decode_info;
        uint8_t* const file;
        uint32_t file_size; // Size of log file in byte
        uint32_t file_entries; // How many log entries have been added to the log files
        uint8_t* const metafile;
        uint8_t* const indexfile;
        uint32_t indexfile_size; // Size of index file in bytes

        // Queues
        uint8_t* data_queue;
        uint8_t* double_buffer;
        uint32_t queue_len; // How many bytes have been added to the queue

        // Queues
        // Write len bytes to queue from var_address
        void write_to_queue(auto var_address, uint8_t len) {
            std::memcpy(this->data_queue + this->queue_len, var_address, len);
            this->queue_len += len;
        }

        // Switch data and double buffer
        void switch_buffers() {
            // Switch data queue and double buffer
            uint8_t* temp = this->data_queue;
            this->data_queue = this->double_buffer;
            this->double_buffer = temp;

            // Write data to file
            this->write_to_file(this->queue_len);
        }

        // Files
        void deserialize_meta_info(uint8_t* metafile) {
            uint8_t* metafile_pos = metafile;

            std::memcpy(&(this->decode_info), metafile_pos, sizeof(this->decode_info));
            metafile_pos += sizeof(this->decode_info);
            std::memcpy(&(this->file_entries), metafile_pos, sizeof(this->file_entries));
            metafile_pos += sizeof(this->file_entries);
            std::memcpy(&(this->file_size), metafile_pos, sizeof(this->file_size));
            metafile_pos += sizeof(this->file_size);
            std::memcpy(&(this->indexfile_size), metafile_pos, sizeof(this->indexfile_size));
            metafile_pos += sizeof(this->indexfile_size);
        }

        uint8_t* serialize_meta_info() {
            uint8_t* metafile_pos = this->metafile;

            std::memcpy(metafile_pos, &(this->decode_info), sizeof(this->decode_info));
            metafile_pos += sizeof(this->decode_info);
            std::memcpy(metafile_pos, &(this->file_entries), sizeof(this->file_entries));
            metafile_pos += sizeof(this->file_entries);
            std::memcpy(metafile_pos, &(this->file_size), sizeof(this->file_size));
            metafile_pos += sizeof(this->file_size);
            std::memcpy(metafile_pos, &(this->indexfile_size), sizeof(this->indexfile_size));
            metafile_pos += sizeof(this->indexfile_size);

            return this->metafile;
        }

        // Dummy
        void write_to_index(time_t timestamp, uint32_t location) {
            uint8_t* indexfile_pos = this->indexfile + this->indexfile_size;

            std::memcpy(indexfile_pos, &timestamp, sizeof(time_t));
            std::memcpy(indexfile_pos + sizeof(time_t), &location, sizeof(uint32_t));

            this->indexfile_size += INDEX_ENTRY_SIZE;
        }

        // Write <size> bytes from active data buffer to log file
        // Dummy
        void write_to_file(uint32_t size) {
            std::memcpy(this->file + this->file_size, this->double_buffer, size);
            // A size worth of bytes was appended to the log file
            this->file_size += size;
        }

        // Initialize the log with the given file (no meta- and indexfile)
        BaseLog(uint8_t* file)
            : file(file)
            , file_size{0}
            , file_entries{0}
            , metafile(NULL)
            , indexfile(NULL)
            , indexfile_size{0}
            , queue_len{0}
        {}

        // Initialize the log (from a metafile) held in file pointed to by the third argument
        BaseLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file)
            : file(file)
            , metafile(metafile)
            , indexfile(indexfile)
            , queue_len{0}
        {
            this->deserialize_meta_info(this->metafile);
        }

    public:
        // Get size of log file in bytes
        uint32_t get_file_size() {
            return this->file_size;
        }

        // Write current state to metafile
        void save_meta_info() {
            if (this->metafile == NULL) return;
            this->serialize_meta_info();
        }
};

template <class T>
class RegularLog : public BaseLog<T> {
    protected:
        // Logic
        time_t last_timestamp; // Last timestamp of logged data
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
            this->write_to_queue(&(this->last_timestamp), sizeof(time_t));
        }

        // Write datapoint in byte form to the data queue
        void write_to_queue_datapoint(T& datapoint) {
            this->write_to_queue(&datapoint, sizeof(T));
        }

        // Write timedelta in byte form to the data queue
        void write_to_queue_timedelta(uint8_t timedelta) {
            this->write_to_queue(&timedelta, sizeof(uint8_t));
        }

        // Write current data_added in byte form to the data queue
        void write_to_queue_data_added() {
            this->write_to_queue(&(this->data_added), sizeof(uint8_t));
            this->switch_buffers();
        }

        // Manually close the current entry (write entry to file)
        void end_entry() {
            this->write_to_queue_data_added();
            // A new entry was written to the log file
            this->file_entries++;

            // Create an index entry (write it to indexfile) if enough log entries have been added to the log file
            if (this->indexfile != NULL && this->file_entries % INDEX_DENSITY == 1) {
                this->write_to_index(this->last_timestamp, this->file_size - this->queue_len);
            }

            // Reset queue length to 0
            this->queue_len = 0;
        }

    public:
        // Initialize the log with the given file (no meta- and indexfile)
        RegularLog(uint8_t* file, int8_t resolution)
            : BaseLog<T>(file)
            , last_timestamp{0}
            , data_added{0}
            , resolution{resolution}
        {
            this->data_queue = (uint8_t *)pvPortMalloc(this->min_queue_size());
            this->double_buffer = (uint8_t *)pvPortMalloc(this->min_queue_size());
        }

        // Initialize the log (from a metafile) held in file pointed to by the third argument
        RegularLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file, int8_t resolution)
            : BaseLog<T>(metafile, indexfile, file)
            , last_timestamp{0}
            , data_added{0}
            , resolution{resolution}
        {
            this->data_queue = (uint8_t *)pvPortMalloc(this->min_queue_size());
            this->double_buffer = (uint8_t *)pvPortMalloc(this->min_queue_size());
        }

        // Log data (implemented differently for different types), attach timestamp in function
        void log(T& data);

        // Log data with a given timestamp in the file
        void log(T& data, time_t timestamp) {
            bool new_entry = false;

            if (!this->last_timestamp
                || this->scale_timedelta(timestamp - this->last_timestamp) >= (1 << (sizeof(uint8_t) * CHAR_BIT))
                || this->data_added >= (1 << (sizeof(uint8_t) * CHAR_BIT)) - 1) {
                // If one timestamp resolution is full, write to queue how many datapoints were recorded under the previous timestamp
                if (this->last_timestamp) {
                    this->end_entry();
                }
                // Create a new timestamp and write it to queue
                this->last_timestamp = timestamp;
                this->write_to_queue_timestamp();
                // This is a new entry with a new timestamp
                new_entry = true;
            }

            // Write datapoint to queue
            this->write_to_queue_datapoint(data);
            // If time_diff was bigger than 8 bits, then a new entry was created above, so this cast is safe
            uint8_t time_diff = (uint8_t) this->scale_timedelta(timestamp - this->last_timestamp);
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

        // Write all datapoints in volatile memory to file
        void flush() {
            this->end_entry();
            this->last_timestamp = 0;
        }

        // Read an array of log entries from the chosen time period
        LogSlice<RegularLog,T> slice(time_t start_ts, time_t end_ts) {
            return log_slice<RegularLog,T>(this->file, this->file_size, this->indexfile, this->indexfile_size, start_ts, end_ts, this->resolution);
        }

        // Log<T> compress(compression_method_t method); // Compress log with the chosen method
        // Log<T> merge(Log<T> otherLog); // Merge two logs and create a new log

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
        static time_t find_last_timestamp(uint8_t* file, uint32_t file_size) {
            uint8_t last_data_added;
            std::memcpy(&last_data_added, file + file_size - sizeof(uint8_t), sizeof(uint8_t));

            uint8_t last_timedelta;
            std::memcpy(&last_timedelta, file + file_size - 2 * sizeof(uint8_t), sizeof(uint8_t));

            time_t last_ts;
            std::memcpy(&last_ts, file + file_size - sizeof(uint8_t) - (last_data_added + 1)*(sizeof(T) + sizeof(uint8_t)) - sizeof(time_t), sizeof(time_t));
            last_ts += last_timedelta;
            return last_ts;
        }

        static RegularLog<T> sliceToLog(uint8_t* new_file, uint8_t* file, uint32_t start_location, uint32_t end_location, int8_t resolution) {
            std::memcpy(new_file, file + start_location, end_location - start_location);
            return RegularLog<T>(new_file, resolution);
        }
};

template <class T>
class SimpleLog : public BaseLog<T> {
    public:
        SimpleLog(uint8_t* file)
            : BaseLog<T>(file)
        {
            this->data_queue = (uint8_t *)pvPortMalloc(sizeof(time_t) + sizeof(T));
            this->double_buffer = (uint8_t *)pvPortMalloc(sizeof(time_t) + sizeof(T));
        }

        SimpleLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file)
            : BaseLog<T>(metafile, indexfile, file)
        {
            this->data_queue = (uint8_t *)pvPortMalloc(sizeof(time_t) + sizeof(T));
            this->double_buffer = (uint8_t *)pvPortMalloc(sizeof(time_t) + sizeof(T));
        }

        // Log data (implemented differently for different types), attach timestamp in function
        void log(T& data);

        void log(T& data, time_t timestamp) {
            this->write_to_queue(&timestamp, sizeof(time_t));
            this->write_to_queue(&data, sizeof(T));

            this->switch_buffers();
            this->file_entries++;

            // If enough data has been logged for a new index entry, create it
            if (this->indexfile != NULL && this->file_entries % INDEX_DENSITY == 1) {
                this->write_to_index(timestamp, this->file_size - sizeof(time_t) - sizeof(T));
            }

            this->queue_len = 0;
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
        static time_t find_last_timestamp(uint8_t* file, uint32_t file_size) {
            time_t last_ts;
            std::memcpy(&last_ts, file + file_size - sizeof(T) - sizeof(time_t), sizeof(time_t));
            return last_ts;
        }

        static SimpleLog<T> sliceToLog(uint8_t* new_file, uint8_t* file, uint32_t start_location, uint32_t end_location, int8_t resolution) {
            std::memcpy(new_file, file + start_location, end_location - start_location);
            return SimpleLog<T>(new_file);
        }
};

template <class T>
class PeriodicLog : public BaseLog<T> {

};

template <class T>
class CircularLog : public RegularLog<T> {

};

// Return the locations of the entries containing start timestamp and end timestamp in the log file in an array
template <template <class> class T, class E>
LogSlice<T,E> log_slice(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts, int8_t resolution);

#endif
