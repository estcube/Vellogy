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

// Task list
// 1. Logi decoder refactor + test siinusandmetega
// 1.5. Log object tagastab logi metadata, kus on lisaks decode infole internal state (kui toimub powerout). Logi konstruktor võtab pointeri metafailile.
// 2. Ettevalmistused failisüsteemi tulekuks
// 3. Logi sisukord (iga 10 TS tagant kirje). Kujul timestamp-tema aadress failis(mitmes bait). Optional (kui metafaili ei anta, ei tehta).
// Hoitakse nii Log objekti väljana kui metafailina (peab säilima pärast powerouti).
// 4. CRC iga logi entry kohta. Optional. Otsi CRCde kohta - mis on meile parim? Mathias pakub CRC8. Triple buffer???
// 5. Slicing. Virtuaalsed sliced. Pointer algustimestampile ja pointer lõputimestampile (lähimale eelnevale). Dokumentatsioon! SliceLog tüüp?
// 6. SliceLog objektil on compress funktsioon. Loetakse antud aegadega piiratud andmed ja siis compressitakse. Kui kasutaja annab faili, kirjutatakse tulemus faili.

// Size of data queue in bytes (when the queue is full, it's written to memory)
// NB! If using FLASH to store logs, this should be equal to (sub)sector size for maximum effective memory usage
#define QUEUE_SIZE 16
// Byte size in bits
#define BYTE_SIZE 8

enum log_element_t {
    TIMESTAMP,
    DATAPOINT,
    TIMEDELTA,
    DATA_ADDED
};

enum compression_method_t {
    LOG_COMPRESSION_FFT,
    LOG_COMPRESSION_HUFFMAN
};

// These structs are for reference only
struct log_decode_info_t {
    uint32_t type; // What datatype???
    uint8_t TS_seconds_size; // For timestamp, we only need the number of bytes to deduce the formatstring
    uint8_t TS_subseconds_size;
    uint8_t U_size; // For timestamp delta, we only need the number of bytes to deduce the formatstring
    uint8_t* T_formatstring;
    uint8_t T_formatstring_len;
    uint32_t file_size; // In bytes, why necessary?
    uint8_t* path;
    uint8_t path_len;
    uint32_t buffer_size; // In bytes, why necessary?
};

struct log_internal_state_t {
    time_t last_timestamp = 0; // Last timestamp of logged data
    // uint32_t data_added;
    uint32_t data_added_file; // Datapoints added under last timestamp in the log file
    uint32_t decode_start_position_file; // In case of a value split between non-volatile and volatile memory, we need to know from which byte does the last whole datapoint (value + time delta) start in the file
    bool flushed = false;
};

struct log_metadata_t {
    log_decode_info_t decode_info;
    log_internal_state_t internal_state;
};

template <class T, class U>
class Log {
    private:
        uint8_t* file;
        uint8_t* metafile;
        uint32_t decode_info_offset = 0;

        // TODO: end log with how many datapoints under last queue item?
        uint8_t* data_queue;
        uint8_t* double_buffer;
        uint32_t queue_len = 0; // How many bytes have been added to the queue

        time_t last_timestamp = 0; // Last timestamp of logged data
        U data_added = 0; // How many datapoints have been added to the queue under the last timestamp
        U data_added_file = 0; // How many datapoints have been added to the file under the last timestamp (in file)
        uint32_t decode_start_position_file = 0; // NB! This needs to be taken into account when doing write_to_file NB! The actual location in the file is N*QUEUE_SIZE + decode_start_position_file
        uint32_t decode_start_position_buffer = 0; // Position of last legitimate datapoint in buffer
        bool flushed = false;

        void deserialize_meta_info(uint8_t* metafile);
        uint8_t* serialize_meta_info();

        void init_metafile(); // Initialize the metafile
        void init_file(); // Initialize the file holding the logs
        void write_to_queue(auto var_address, uint8_t len, log_element_t element_type); // Write data with specified length in byte form to the data queue
        void write_to_file(uint32_t size); // Write <size> bytes from active data buffer to log file
        void switch_buffers(); // Switch data and double buffer

    public:
        Log(); // Initialize the log
        Log(uint8_t* metafile, uint8_t* file); // Initialize the log (from a metafile) held in file pointed to by the second argument
        void log(T* data); // Log data (implemented differently for different types), attach timestamp in function
        void log(T* data, time_t timestamp); // Log data with a given timestamp in the file

        void flush_buffer(); // Write all things in buffer to memory
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
