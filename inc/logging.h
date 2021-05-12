#ifndef LOGGING_H
#define LOGGING_H

// C++ includes
#include <vector>

// C includes
#include <time.h>
#include <stdint.h>

// Smaller stuff:
// TODO: use references
// TODO: mis juhtub, kui entry saab täis, aga resolution pole veel täis? Kas see on võimalik?
// TODO: flush funktsioon tagasi tuua

// NOTE: metafile ülekirjutamine on ok

// Task list
// 1. Logi decoder refactor + test siinusandmetega
// 1.5. Log object tagastab logi metadata, kus on lisaks decode infole internal state (kui toimub powerout). Logi konstruktor võtab pointeri metafailile.
// 2. Ettevalmistused failisüsteemi tulekuks
// 3. Logi sisukord (iga 10 TS tagant kirje). Kujul timestamp-tema aadress failis(mitmes bait). Optional (kui metafaili ei anta, ei tehta).
// Hoitakse nii Log objekti väljana kui metafailina (peab säilima pärast powerouti).
// 4. CRC iga logi entry kohta. Optional. Otsi CRCde kohta - mis on meile parim? Mathias pakub CRC8. Triple buffer???
// 5. Slicing. Virtuaalsed sliced. Pointer algustimestampile ja pointer lõputimestampile (lähimale eelnevale). Dokumentatsioon! SliceLog tüüp?
// 6. SliceLog objektil on compress funktsioon. Loetakse antud aegadega piiratud andmed ja siis compressitakse. Kui kasutaja annab faili, kirjutatakse tulemus faili.

// Byte size in bits
#define BYTE_SIZE 8
// After how many log entries is an index object created
#define INDEX_DENSITY 5
// How much entries can one index hold
#define INDEX_SIZE 16

enum compression_method_t {
    LOG_COMPRESSION_FFT,
    LOG_COMPRESSION_HUFFMAN
};

// While testing, we want to keep this short to improve readability
#define STRING_SIZE 4

struct log_decode_info_t {
    uint32_t type; // Log semantic data type (what type of sensor, for example)
    uint32_t buffer_size;
    uint32_t file_size;
    uint8_t path[STRING_SIZE];
    uint8_t T_formatstring[STRING_SIZE];
    uint8_t TS_seconds_size; // For timestamp, we only need the number of bytes to deduce the formatstring
    uint8_t TS_subseconds_size;
    uint8_t U_size; // For timestamp delta, we only need the number of bytes to deduce the formatstring
};

template <class T, class U>
class Log {
    private:
        uint8_t* file;
        uint8_t* metafile;
        log_decode_info_t decode_info;

        uint32_t file_size = 0; // Size of log file in bytes // TODO: this logic really shouldn't be on the logging level
        uint32_t file_entries = 0; // How many log entries have been added to the log file

        time_t index_ts[INDEX_SIZE];
        uint32_t index_pos[INDEX_SIZE];
        uint32_t index_entries = 0; // How many entries have been added to the index

        // TODO: end log with how many datapoints under last queue item?
        uint8_t* data_queue;
        uint8_t* double_buffer;
        uint32_t queue_len = 0; // How many bytes have been added to the queue

        time_t last_timestamp = 0; // Last timestamp of logged data
        U data_added = 0; // How many datapoints have been added to the queue under the last timestamp

        uint32_t min_queue_size(); // Calculate minimum possible size in bytes for the data queue (maximum size of log in bytes)

        void write_to_queue(auto var_address, uint8_t len); // Write len bytes to queue from var_address
        void write_to_queue_timestamp(); // Write current timestamp in byte form to the data queue
        void write_to_queue_datapoint(T* datapoint); // Write datapoint in byte form to the data queue
        void write_to_queue_timedelta(U timedelta); // Write timedelta in byte form to the data queue
        void write_to_queue_data_added(); // Write current data_added in byte form to the data queue
        void switch_buffers(); // Switch data and double buffer

        void deserialize_meta_info(uint8_t* metafile);
        uint8_t* serialize_meta_info();
        void init_metafile(); // Initialize the metafile
        void init_file(); // Initialize the file holding the logs
        void write_to_file(uint32_t size); // Write <size> bytes from active data buffer to log file

    public:
        Log(); // Initialize the log
        Log(uint8_t* metafile, uint8_t* file); // Initialize the log (from a metafile) held in file pointed to by the second argument
        void log(T* data); // Log data (implemented differently for different types), attach timestamp in function
        void log(T* data, time_t timestamp); // Log data with a given timestamp in the file

        void save_meta_info(); // Write current state to metafile

        Log<T,U> slice(time_t starttime, time_t endtime); // Read an array of log entries from the chosen time period
        Log<T,U> compress(compression_method_t method); // Compress log with the chosen method
        Log<T,U> merge(Log<T,U> otherLog); // Merge two logs and create a new log
};

template <class T, class U>
class PeriodicLog : public Log<T,U> {

};

template <class T, class U>
class CircularLog : public Log<T,U> {

};

template <class T, class U>
class ThreadsafeLog : public Log<T,U> {

};

template class Log<int, uint8_t>;
// template class Log<int, uint16_t>;
// template class Log<int, uint32_t>;
template class Log<double, uint8_t>;
// template class Log<double, uint16_t>;
// template class Log<double, uint32_t>;
#endif
