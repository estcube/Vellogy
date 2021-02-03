#ifndef LOGGING_H
#define LOGGING_H

// C++ includes
#include <vector>

// C includes
#include <time.h>
#include <stdint.h>

// TODO: use references
// TODO: mis juhtub, kui entry saab täis, aga resolution pole veel täis? Kas see on võimalik?

// Task list
// 1. Logi decoder + flush funktsioon + test siinusandmetega
// 2. Ettevalmistused failisüsteemi tulekuks
// 3. Logi sisukord (iga 10 TS tagant kirje). Kujul timestamp-tema aadress failis(mitmes bait). Optional (kui metafaili ei anta, ei tehta).
// Hoitakse nii Log objekti väljana kui metafailina (peab säilima pärast powerouti).
// 4. CRC iga logi entry kohta. Optional. Otsi CRCde kohta - mis on meile parim? Mathias pakub CRC8. Triple buffer???
// 5. Slicing. Virtuaalsed sliced. Pointer algustimestampile ja pointer lõputimestampile (lähimale eelnevale). Dokumentatsioon! SliceLog tüüp?
// 6. SliceLog objektil on compress funktsioon. Loetakse antud aegadega piiratud andmed ja siis compressitakse. Kui kasutaja annab faili, kirjutatakse tulemus faili.

// Size of data queue in bytes (when the queue is full, it's written to memory)
// NB! If using FLASH to store logs, this should be equal to (sub)sector size for maximum effective memory usage
#define QUEUE_SIZE 256
// Byte size in bits
#define BYTE_SIZE 8

enum compression_method_t {
    LOG_COMPRESSION_FFT,
    LOG_COMPRESSION_HUFFMAN
};

template<typename T, typename U>
struct datapoint_t {
    T data; // Datapoint value
    U delta; // Difference (in ms) from the timestamp
};

template <class T, class U>
class Log {
    private:
        // TODO: end log with how many datapoints under last queue item?
        uint8_t* data_queue;
        uint8_t* double_buffer;
        uint32_t queue_len = 0; // How many bytes have been added to the queue
        time_t last_timestamp = 0; // Last timestamp of logged data
        U data_added = 0; // How many datapoints have been added to the queue under the last timestamp
        bool flushed = false;

        void init_file(); // Initialize the file holding the logs
        void write_to_queue(auto var_address, uint8_t len); // Write data with specified length in byte form to the data queue
        void write_to_file(uint32_t size); // Write <size> bytes from active data buffer to log file
        void switch_buffers(); // Switch data and double buffer

    public:
        Log(); // Initialize the log
        void log(T* data); // Log data (implemented differently for different types), attach timestamp in function
        void log(T* data, time_t timestamp); // Log data with a given timestamp in the file

        void flush_buffer(); // Write all things in buffer to memory

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

// template class Log<int, int>;
template class Log<int, uint8_t>;
// template class Log<int, uint16_t>;
// template class Log<int, uint32_t>;
#endif
