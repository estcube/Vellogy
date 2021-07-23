/**
 * @file logutility.hpp
 *
 * This file provides the LogSlice class
 * used for addressing datapoints in the log that fall into a specific time interval
 */

#ifndef LOGSLICE_H
#define LOGSLICE_H

#include "baselog.hpp"

/**
 * Class denoting a valid subset of a log file, called slice
 *
 * @tparam T the type of log held in the file being sliced, a subclass of BaseLog
 * @tparam E the datatype of datapoints held in the log file being sliced
 */
template <template <class> class T, class E>
class LogSlice {
    private:
        uint8_t* file;              ///< Log file being sliced
        uint32_t start_location;    ///< Beginning of slice in the log file
        uint32_t end_location;      ///< End of slice in the log file
        int8_t resolution;          ///< Resolution of timestamps in the log file

    public:
        /**
         * Create a new log slice
         */
        LogSlice(
            uint8_t* file,              ///< [in] Log file being sliced
            uint32_t start_location,    ///< [in] Beginning of slice in the log file
            uint32_t end_location,      ///< [in] End of slice in the log file
            int8_t resolution           ///< [in] Resolution of timestamps in the log file
        )
            : file(file)
            , start_location{start_location}
            , end_location{end_location}
            , resolution{resolution}
        {}

        /**
         * Return the file being sliced
         */
        uint8_t* get_file() {
            return this->file;
        }

        /**
         * Return the beginning of the slice
         */
        uint32_t get_start_location() {
            return this->start_location;
        }

        /**
         * Return the end of the slice
         */
        uint32_t get_end_location() {
            return this->end_location;
        }

        /**
         * Return the resolution of timestamps in the log file
         */
        int8_t get_resolution() {
            return this->resolution;
        }

        /**
         * Copy the log slice into a new log file and create a Log object of it
         * NB! User is responsible for deleting the log object inside the log interface object returned by this function
         */
        Log<T,E> createLog(
            uint8_t* new_file   ///< [in] File where the log slice will be copied to
        )  {
            // Allocate memory for new Log
            uint8_t* buf = (uint8_t *)pvPortMalloc(sizeof(T<E>));
            // Copy slice contents into new_file and return the new Log held in new_file
            T<E>* new_log = new(buf) T<E>(this, new_file);
            return Log<T,E>(new_log);
        }
};

#endif
