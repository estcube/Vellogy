#ifndef LOGUTILITY_H
#define LOGUTILITY_H

#include "log.h"

// Return the locations of the entries containing start timestamp and end timestamp in the log file in an array
template <template <class> class T, class E>
LogSlice<T,E> log_slice(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts, int8_t resolution);

#endif
