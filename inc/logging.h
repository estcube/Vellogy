#ifndef LOGGING_H
#define LOGGING_H

// C++ includes
#include <vector>

// C includes
#include <time.h>

// Maximum number of data points in one entry
#define BLOCK_SIZE 4

enum compression_method_t {
    LOG_COMPRESSION_FFT,
    LOG_COMPRESSION_HUFFMAN
};

template <typename T>
struct entry_t {
    time_t timestamp;
    entry_t* previous;
};

template <typename T>
struct simple_entry_t : entry_t<T> {
    T data;
};

template <typename T>
struct periodic_entry_t : entry_t<T> {
    time_t endTimestamp;
    time_t interval;
    T data[BLOCK_SIZE];
};

template <typename T>
struct multi_entry_t : entry_t<T> {
    T data[BLOCK_SIZE];
    T deltas[BLOCK_SIZE];
    int offset; // How many data points have been added to the entry?
};

template <class T>
class Log {
    void* file; // Pointer to the log file
    int filesize; // Log file size in bytes
    multi_entry_t<T>* last_entry;

    public:
        Log(void* file); // Create the log in a given file location
        Log(void* file, int size); // Create the log in a given file with a given size

        void log(T* data); // Log data (implemented differently for different types), attach timestamp in function
        void log(T* data, time_t timestamp); // Log data with a given timestamp in the file

        // log_entry_t* read(time_t starttime, time_t endtime); // Read an array of log entries from the chosen time period

        Log<T> slice(time_t starttime, time_t endtime);
        Log<T> compress(compression_method_t method); // Compress log with the chosen method

        // Log<T> compress_interval(compression_method_t method, time_t starttime, time_t endtime); // Compress a time interval from the log with the chosen method

        Log<T> merge(Log<T> otherLog); // Merge two logs and create a new log
};

template <class T>
class PeriodicLog : public Log<T> {

};

template <class T>
class CircularLog : public Log<T> {

};

template class Log<int>;
#endif
