/**
 * @file baselog.hpp
 *
 * Author: Annika Jaakson
 * Refactored by: Kerdo Kurs
 *
 * This file provides the base class of all types of logs
 * and common enums and structs for all other log classes
 */

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

namespace eclog {

/**
 * Log file structure types
 */
enum log_file_type_t : uint8_t {
    LOG_REGULAR = 0,    ///< Corresponds to RegularLog class
    LOG_SIMPLE,         ///< Corresponds to SimpleLog class
    LOG_PERIODIC,       ///< Corresponds to PeriodicLog class
    LOG_CIRCULAR        ///< Corresponds to CircularLog class
};

/**
 * Datatypes that a log can hold
 * TODO: make the list complete
 */
enum log_data_type_t : uint8_t {
    LOG_UINT8_T = 0,    ///< C++ datatype uint8_t
    LOG_UINT16_T,       ///< C++ datatype uint16_t
    LOG_UINT32_T,       ///< C++ datatype uint32_t
    LOG_INT8_T,         ///< C++ datatype int8_t
    LOG_INT16_T,        ///< C++ datatype int16_t
    LOG_INT32_T,        ///< C++ datatype int32_t
    LOG_FLOAT,          ///< C++ datatype float
    LOG_DOUBLE          ///< C++ datatype double
};

/**
 * Compression methods for sending log files to the ground
 */
enum log_compression_method_t : uint8_t {
    LOG_COMPRESSION_FFT,        ///< Compression by Fast Fourier Transform
    LOG_COMPRESSION_HUFFMAN     ///< Compressoon by Huffman coding
};

/**
 * Optional log file decode info held in the metafile
 */
struct log_decode_info_t {
    uint32_t type;                                  ///< Log semantic data type (what type of sensor, for example)
    uint8_t path[LOG_PATH_LEN];                     ///< Path to the log file in the filesystem
    uint8_t T_formatstring[LOG_FORMATSTRING_LEN];   ///< Format string for decoding datapoints
    // For timestamp, we only need the number of bytes to deduce the formatstring
    uint8_t TS_seconds_size;                        ///< How many bytes are allocated in EC-2 timestamps for seconds
    uint8_t TS_subseconds_size;                     ///< How many bytes are allocated in EC-2 timestamps for subseconds
};

// Forward declarations of different classes needed for logging
template <class T> class base_log;
template <class T> class regular_log;
template <class T> class simple_log;

template<template <class> class T, class E>
concept loggable = std::is_convertible<T<E>, base_log<E>>::value;

template <template <class> class T, class E> class log_slice;
template<template <class> class T, class E> requires loggable<T,E> class log;

/**
 * A base class for different types of logs
 *
 * This class provides log file and metafile writing/reading functionality
 * plus buffer logic that is common for all types of logs
 *
 * @tparam T datatype of datapoints held in the log
 */
template <class T>
class base_log {
    protected:
        // Files
        log_decode_info_t decode_info;  ///< Log optional decode info
        uint8_t* const file;            ///< Pointer to the log file
        uint32_t file_size;             ///< Size of the log file in bytes
        uint32_t entries_added;         ///< How many log entries have been added to the log file during current logging session
        uint8_t* const metafile;        ///< Pointer to the log metafile (can be NULL)
        uint8_t* const indexfile;       ///< Pointer to the log index file (can be NULL)
        uint32_t indexfile_size;        ///< Size of the log index file in bytes

        // Queues
        uint8_t* data_queue;    ///< Primary buffer where datapoints accumulate before writing them to the log file
        uint8_t* double_buffer; ///< Secondary buffer (primary and secondary buffers are switched when primary gets full)
        uint32_t queue_len;     ///< How many bytes have been added to the data queue/primary buffer

        /**** Queue functions ****/

        /**
         * Write len bytes to data queue from var_address
         */
        void write_to_queue(
            auto var_address,   ///< [in] From where will data be written to the queue
            uint8_t len         ///< [in] How many bytes from var_address will be written to the queue
        ) {
            std::memcpy(this->data_queue + this->queue_len, var_address, len);
            this->queue_len += len;
        }

        /**
         * Switch data and double buffer
         */
        void switch_buffers() {
            // Switch data queue and double buffer
            uint8_t* temp = this->data_queue;
            this->data_queue = this->double_buffer;
            this->double_buffer = temp;

            // Write data to file
            this->write_to_file(this->queue_len);
        }

        /**** File functions ****/

        /**
         * Read metainfo into the log object from the metafile
         */
        void deserialize_meta_info(
            uint8_t* metafile   ///< [in] Pointer to the metafile that holds the info to be deserialized
        ) {
            uint8_t* metafile_pos = metafile;

            std::memcpy(&(this->decode_info), metafile_pos, sizeof(this->decode_info));
            metafile_pos += sizeof(this->decode_info);
            std::memcpy(&(this->file_size), metafile_pos, sizeof(this->file_size));
            metafile_pos += sizeof(this->file_size);
            std::memcpy(&(this->indexfile_size), metafile_pos, sizeof(this->indexfile_size));
            metafile_pos += sizeof(this->indexfile_size);
        }

        /**
         * Write metainfo ifrom the log object into the metafile
         */
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

        /**
         * Write a new index entry into the index file
         * Dummy
         */
        void write_to_index(
            time_t timestamp,   ///< [in] Timestamp in the beginning of a log entry
            uint32_t location   ///< [in] Location of the log entry in the log file
        ) {
            uint8_t* indexfile_pos = this->indexfile + this->indexfile_size;

            std::memcpy(indexfile_pos, &timestamp, sizeof(time_t));
            std::memcpy(indexfile_pos + sizeof(time_t), &location, sizeof(uint32_t));

            this->indexfile_size += LOG_INDEX_ENTRY_SIZE;
        }

        /**
         * Write bytes from active data buffer to log file
         * Dummy
         */
        void write_to_file(
            uint32_t size   ///< [in] How many bytes to write form data queue to log file
        ) {
            std::memcpy(this->file + this->file_size, this->double_buffer, size);
            // A size worth of bytes was appended to the log file
            this->file_size += size;
        }

        /**
         * Initialize the log with the given file (no meta- and indexfile)
         */
        base_log(
            uint8_t* file   ///< [in] Pointer to the file where datapoints will be saved
        )
            : file(file)
            , file_size{0}
            , entries_added{0}
            , metafile(NULL)
            , indexfile(NULL)
            , indexfile_size{0}
            , queue_len{0}
        {}

        /**
         * Initialize the log (from a metafile) held in the given file
         */
        base_log(
            uint8_t* metafile,  ///< [in] Pointer to the file where metainfo will be saved
            uint8_t* indexfile, ///< [in] Pointer to the file where index entries will be saved
            uint8_t* file       ///< [in] Pointer to the file where datapoints will be saved
        )
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

        /**
         * Return the decode info of the log file
         */
        log_decode_info_t get_decode_info() {
            return this->decode_info;
        }

        /**
         * Return pointer to the log file
         */
        uint8_t* get_file() {
            return this->file;
        }

        /**
         * Return pointer to the log index file
         */
        uint8_t* get_indexfile() {
            return this->indexfile;
        }

        /**
         * Return pointer to the log metafile
         */
        uint8_t* get_metafile() {
            return this->metafile;
        }

        /**
         * Return size of log file in bytes
         */
        uint32_t get_file_size() {
            return this->file_size;
        }

        /**
         * Return size of the log index file in bytes
         */
        uint32_t get_indexfile_size() {
            return this->indexfile_size;
        }

        /**** Utility functions ****/

        /**
         * Write current log state to metafile (valid for all)
         */
        void save_meta_info() {
            if (this->metafile == NULL) return;
            this->serialize_meta_info();
        }
};

}

#endif
