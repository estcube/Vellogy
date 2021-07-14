#ifndef LOGSLICE_H
#define LOGSLICE_H

#include "baselog.h"

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

        Log<T,E> createLog(uint8_t* new_file)  {
            // TODO: allocate mem
            T<E> new_log = T<E>::sliceToLog(new_file, this->file, this->start_location, this->end_location, this->resolution);
            return Log<T,E>(&new_log);
        }
};

#endif
