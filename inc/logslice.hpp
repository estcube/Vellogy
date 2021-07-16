#ifndef LOGSLICE_H
#define LOGSLICE_H

#include "baselog.hpp"

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

        // NB! User is responsible for deleting the log object inside the log interface object returned by this function
        Log<T,E> createLog(uint8_t* new_file)  {
            // Allocate memory for new Log
            uint8_t* buf = (uint8_t *)pvPortMalloc(sizeof(T<E>));
            // Copy slice contents into new_file and return the new Log held in new_file
            T<E>* new_log = new(buf) T<E>(this, new_file);
            return Log<T,E>(new_log);
        }
};

#endif
