#ifndef LOGGING_H
#define LOGGING_H

// C++ includes
#include <cstring>
#include <climits>
#include <ctime>
#include <cstdint>
#include <utility>
#include <cmath>

// C includes
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
// TODO: getter resolution jaoks
// TODO: private struct find_entry_location jaoks
// TODO: SimpleLog indeks korda

// TODO: väiksemad asjad (üleval), 1 suurem (all)

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
    uint8_t U_size; // For timestamp delta, we only need the number of bytes to deduce the formatstring
};

struct log_slice_t {
    uint8_t* file = NULL;
    uint32_t start_location = UINT_MAX;
    uint32_t end_location = UINT_MAX;
};

template <class T> class BaseLog;
template <class T> class RegularLog;
template <class T> class LogSlice;

template <class T>
class LogSlice {
    private:
        uint8_t* const file;
        const uint32_t start_location;
        const uint32_t end_location;
        const int8_t resolution;

    public:
        LogSlice(uint8_t* file, uint32_t start_location, uint32_t end_location, int8_t resolution);
        uint8_t* get_file();
        uint32_t get_start_location();
        uint32_t get_end_location();
        int8_t get_resolution();
        BaseLog<T> createLog(uint8_t* new_file);
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
        void write_to_queue(auto var_address, uint8_t len); // Write len bytes to queue from var_address
        void switch_buffers(); // Switch data and double buffer

        // Files
        void deserialize_meta_info(uint8_t* metafile);
        uint8_t* serialize_meta_info();
        void write_to_index(time_t timestamp, uint32_t location);
        void write_to_file(uint32_t size); // Write <size> bytes from active data buffer to log file

        BaseLog(uint8_t* file); // Initialize the log with the given file (no meta- and indexfile)
        BaseLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file); // Initialize the log (from a metafile) held in file pointed to by the third argument

    public:
        uint32_t get_file_size(); // Get size of log file in bytes
        void save_meta_info(); // Write current state to metafile
};

template <class T>
class RegularLog : public BaseLog<T> {
    protected:
        // Logic
        time_t last_timestamp; // Last timestamp of logged data
        uint8_t data_added; // How many datapoints have been added to the queue under the last timestamp
        const int8_t resolution; // 10^resolution is the smallest unit of time that needs to separable

        uint32_t min_queue_size(); // Calculate minimum possible size in bytes for the data queue (maximum size of log in bytes)
        uint16_t scale_timedelta(uint64_t timedelta); // Round a given timedelta to a multiple of 2^resolution and divide by 2^resolution
        void write_to_queue_timestamp(); // Write current timestamp in byte form to the data queue
        void write_to_queue_datapoint(T& datapoint); // Write datapoint in byte form to the data queue
        void write_to_queue_timedelta(uint8_t timedelta); // Write timedelta in byte form to the data queue
        void write_to_queue_data_added(); // Write current data_added in byte form to the data queue
        void end_entry(); // Manually close the current entry (write entry to file)

    public:
        RegularLog(uint8_t* file, int8_t resolution); // Initialize the log with the given file (no meta- and indexfile)
        RegularLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file, int8_t resolution); // Initialize the log (from a metafile) held in file pointed to by the third argument
        void log(T& data); // Log data (implemented differently for different types), attach timestamp in function
        void log(T& data, time_t timestamp); // Log data with a given timestamp in the file

        void flush(); // Write all datapoints in volatile memory to file

        LogSlice<T> slice(time_t start_ts, time_t end_ts); // Read an array of log entries from the chosen time period
        // Log<T> compress(compression_method_t method); // Compress log with the chosen method
        // Log<T> merge(Log<T> otherLog); // Merge two logs and create a new log
};

template <class T>
class SimpleLog : public BaseLog<T> {
    private:
        uint32_t data_added; // Datapoints added to data_queue
        time_t last_index_ts;
        uint32_t last_index_pos;
    public:
        SimpleLog(uint8_t* file);
        SimpleLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file);
        void log(T& data); // Log data (implemented differently for different types), attach timestamp in function
        void log(T& data, time_t timestamp);

        void flush(); // Write all datapoints in volatile memory to file

        LogSlice<T> slice(time_t start_ts, time_t end_ts); // Read an array of log entries from the chosen time period
};

template <class T>
class PeriodicLog : public BaseLog<T> {

};

template <class T>
class CircularLog : public RegularLog<T> {

};
// Return the locations of the entries containing start timestamp and end timestamp in the log file in an array
template <class T>
LogSlice<T> log_slice(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts, int8_t resolution);

#endif
