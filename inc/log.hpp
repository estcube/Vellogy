/**
 * @file log.hpp
 *
 * Author: Annika Jaakson
 * Refactored by: Kerdo Kurs
 *
 * Module for data logging on ESTCube-2
 * The module allows to:
 * * Create a log in a given file
 * * Save datapoints and their capturing times in the created log file
 * * Search for data logged in a given time interval from the log file
 *
 * Following macros can be defined in logging_cfg.h:
 * * LOG_PATH_LEN                       - How many bytes are allocated for holding the path to a log file
 * * LOG_FORMATSTRING_LEN               - How many bytes are allocated for holding a timestamp or a datapoint formatstring (for decoding)
 * * LOG_REGULAR_INDEX_DENSITY          - After how many log entries is an index entry created (for regular logs)
 * * LOG_SIMPLE_INDEX_DENSITY           - After how many log entries is an index entry created (for simple logs)
 * * LOG_PERIODIC_INDEX_DENSITY         - After how many log entries is an index entry created (for periodic logs)
 * * LOG_PERIODIC_DATAPOINTS_IN_ENTRY   - How many datapoints can one periodic log entry hold
 */

#ifndef LOG_H
#define LOG_H

#include "logslice.hpp"
#include "simplelog.hpp"
#include "regularlog.hpp"
#include "periodiclog.hpp"
#include "circularlog.hpp"
#include "logutility.hpp"

namespace eclog {

/**
 * Log common user interface class
 *
 * The Log object is a wrapper for a BaseLog subclass object.
 * All the Log class methods are invoked on the BaseLog subclass object.
 * This allows to create a common interface for handling logs
 *
 * @tparam T a subclass of BaseLog on which all the functions are actually called (determines the inner log file structure)
 * @tparam E the type of data held in the log
 */
template<template <class> class T, class E> requires loggable<T,E>
class log {
    private:
        void* obj;  ///< Pointer to the BaseLog<E> subclass object of type T<E> that the Log object wraps
    public:
        /**** Constructors ****/

        /**
         * Initialize the Log object from a given BaseLog subclass object
         */
        log(
            T<E>* log_obj   ///< [in] A BaseLog subclass object that the Log methods will be invoked on
        ) {
            this->obj = log_obj;
        };

        /**** Getters ****/

        /**
         * Return pointer to the inner BaseLog subclass object
         */
        void* get_obj() {
            return obj;
        }

        /**
         * Return the decode info of the log file
         */
        log_decode_info_t get_decode_info() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_decode_info();
        }

        /**
         * Return pointer to the log file
         */
        uint8_t* get_file() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_file();
        }

        /**
         * Return pointer to the log index file
         */
        uint8_t* get_indexfile() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_indexfile();
        }

        /**
         * Return pointer to the log metafile
         */
        uint8_t* get_metafile() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_metafile();
        }

        /**
         * Return size of log file in bytes
         */
        uint32_t get_file_size() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_file_size();
        }

        /**
         * Return size of the log index file in bytes
         */
        uint32_t get_indexfile_size() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_indexfile_size();
        }

        /**** Main functionality ****/

        /**
         * Write given datapoint with given timestamp to log file
         */
        void write(
            E& data,            ///< [in] Datapoint to be logged
            time_t timestamp    ///< [in] Capturing time of the datapoint
        ) {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->log(data, timestamp);
        };

        /**
         * Read an array of log entries from the chosen time period
         */
        log_slice<T,E> slice(
            time_t start_ts,    ///< [in] Starting point of the chosen time period
            time_t end_ts       ///< [in] Endpoint of the chosen time period
        ) {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->slice(start_ts, end_ts);
        }

        /**
         * Read an array of log entries from the chosen time period
         * Write resulting log slice into the file new_file
         */
        log_slice<T,E> slice(
            time_t start_ts,    ///< [in] Starting point of the chosen time period
            time_t end_ts,      ///< [in] Endpoint of the chosen time period
            uint8_t* new_file   ///< [in] File where the log slice is to be written
        ) {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->slice(start_ts, end_ts, new_file);
        }

        /**** Utility functions ****/

        /**
         * Write all datapoints in volatile memory to file (valid for everything except SimpleLog)
         */
        void flush() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->flush();
        }

        /**
         * Get resolution of log timestamps (valid only for RegularLog)
         */
        int8_t get_resolution() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->get_resolution();
        }

        /**
         * Signify period change in incoming data (valid only for PeriodicLog)
         */
        void period_change() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            return x->period_change();
        }

        /**
         * Write current log state to metafile (valid for all)
         */
        void save_meta_info() {
            T<E>* x = static_cast<T<E>*>(this->obj);
            x->save_meta_info();
        }
};

}

#endif
