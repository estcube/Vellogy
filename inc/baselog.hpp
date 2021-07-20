#ifndef BASELOG_H
#define BASELOG_H

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

enum log_file_type_t : uint8_t {
    LOG_REGULAR = 0,
    LOG_SIMPLE,
    LOG_PERIODIC
};

enum log_data_type_t : uint8_t {
    LOG_UINT8_T = 0,
    LOG_UINT16_T,
    LOG_UINT32_T,
    LOG_INT8_T,
    LOG_INT16_T,
    LOG_INT32_T,
    LOG_FLOAT,
    LOG_DOUBLE
};

enum log_compression_method_t : uint8_t {
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

template <class T>
class BaseLog {
    protected:
        // Files
        log_decode_info_t decode_info;
        uint8_t* const file;
        uint32_t file_size; // Size of log file in bytes
        uint32_t entries_added; // How many log entries have been added to the log file during current logging session
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
            std::memcpy(&(this->file_size), metafile_pos, sizeof(this->file_size));
            metafile_pos += sizeof(this->file_size);
            std::memcpy(&(this->indexfile_size), metafile_pos, sizeof(this->indexfile_size));
            metafile_pos += sizeof(this->indexfile_size);
        }

        uint8_t* serialize_meta_info() {
            uint8_t* metafile_pos = this->metafile;

            std::memcpy(metafile_pos, &(this->decode_info), sizeof(this->decode_info));
            metafile_pos += sizeof(this->decode_info);
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
            , entries_added{0}
            , metafile(NULL)
            , indexfile(NULL)
            , indexfile_size{0}
            , queue_len{0}
        {}

        // Initialize the log (from a metafile) held in file pointed to by the third argument
        BaseLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file)
            : file(file)
            , entries_added{0}
            , metafile(metafile)
            , indexfile(indexfile)
            , queue_len{0}
        {
            this->deserialize_meta_info(this->metafile);
        }

    public:
        /**** Getters ****/

        log_decode_info_t get_decode_info() {
            return this->decode_info;
        }

        uint8_t* get_file() {
            return this->file;
        }

        uint8_t* get_indexfile() {
            return this->indexfile;
        }

        uint8_t* get_metafile() {
            return this->metafile;
        }

        // Get size of log file in bytes
        uint32_t get_file_size() {
            return this->file_size;
        }

        // Get size of log indexfile in bytes
        uint32_t get_indexfile_size() {
            return this->indexfile_size;
        }

        /**** Utility functions ****/

        // Write current state to metafile
        void save_meta_info() {
            if (this->metafile == NULL) return;
            this->serialize_meta_info();
        }
};

#endif
