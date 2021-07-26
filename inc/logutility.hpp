/**
 * @file logutility.hpp
 *
 * This file provides some standalone utility functions,
 * for example a function for creating slices from a given log file
 */

#ifndef LOGUTILITY_H
#define LOGUTILITY_H

#include "log.hpp"

namespace Logging {

typedef uint32_t (*entry_search_fun)(uint8_t*, uint32_t, uint8_t, time_t, uint32_t, bool);

/**
 * Struct denoting the location of a log entry in the log file and in the log index file
 */
struct entry_location_t {
    uint32_t location_in_file;  ///< Location of log entry in file
    uint32_t index_in_index;    ///< Which index entry contains the log entry
};

/**
 * Find location of a given timestamp in file, using index search
 */
static entry_location_t find_entry_location(
    uint8_t* file,                      ///< [in] Pointer to the log file where we search the entry
    uint32_t file_size,                 ///< [in] Size of the log file
    uint32_t file_break,                ///< [in] Location of the end of the last logged datapoint
    uint8_t* indexfile,                 ///< [in] Pointer to the log index file
    uint32_t indexfile_size,            ///< [in] Size of the log index file
    uint8_t datapoint_size,             ///< [in] Size of datapoints in the log file in bytes
    entry_search_fun find_log_entry,    ///< [in] Function used to search for the log entry when the appropriate index entry has been found
    uint32_t first_index,               ///< [in] Number of the first index entry of the interval the log entry is searched from
    uint32_t second_index,              ///< [in] Number of the second index entry of the interval the log entry is searched from
    time_t timestamp,                   ///< [in] Timestamp to be searched
    bool succeeding                     ///< [in] Do we want the entry containing the timestamp or the entry after it
) {

    uint32_t location;

    // If no binary search is needed
    if (first_index == second_index) {
        // If first_index == second_index, then we have no information about the interval in which the timestamp lies, so we have to scan the whole file
        location = find_log_entry(file, file_size, datapoint_size, timestamp, file_break, succeeding);
        return (entry_location_t) { .location_in_file = location, .index_in_index = first_index };
    }

    // Find timestamp at second_index of indexfile
    time_t second_ts;
    std::memcpy(&second_ts, indexfile + second_index * LOG_INDEX_ENTRY_SIZE, sizeof(time_t));

    while (true) {
        // Divide interval between first_index and second_index into 2 parts
        uint32_t middle_index = (first_index + second_index) / 2;
        // Find index entry timestamp in the middle of the [first_index, second_index] interval
        time_t middle_ts;
        std::memcpy(&middle_ts, indexfile + middle_index * LOG_INDEX_ENTRY_SIZE, sizeof(time_t));
        bool ts_exact_match = timestamp == middle_ts;  // Timestamp happened to be an index entry timestamp (quite unlikely)

        if (second_index - first_index == 1 || ts_exact_match) {  // The binary search is complete
            uint32_t search_index = ts_exact_match ? (succeeding ? middle_index + 1 : middle_index) : (timestamp < second_ts ? second_index : UINT_MAX);

            // If timestamp was greater than or equal to second_ts, start looking for it from the end of the file
            // Otherwise start searching from the location that is saved in indexfile under entry numbered search_index
            uint32_t search_location = file_break;
            if (search_index != UINT_MAX) std::memcpy(&search_location, indexfile + search_index * LOG_INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));

            // If the timestamp happened to be an exact index entry timestamp, the search_location already is the location of the timestamp in the log file
            location = ts_exact_match && !succeeding ? search_location : find_log_entry(file, file_size, datapoint_size, timestamp, search_location, succeeding);
            return (entry_location_t) { .location_in_file = location, .index_in_index = middle_index };
        } else if (timestamp > middle_ts) {
            first_index = middle_index;
        } else if (timestamp < middle_ts) {
            second_index = middle_index;
            std::memcpy(&second_ts, indexfile + second_index * LOG_INDEX_ENTRY_SIZE, sizeof(time_t));
        }
    }
}

/**
 * Return the locations of the entries containing start timestamp and end timestamp in the log file
 *
 * @tparam T the type of log (RegularLog, PeriodicLog, etc)
 * @tparam E the datatype of datapoints held in the log
 */
template <template <class> class T, class E>
LogSlice<T,E> log_slice(
    uint8_t* file,                      ///< [in] Pointer to the log file where we search the entry
    uint32_t file_size,                 ///< [in] Size of the log file
    uint8_t* indexfile,                 ///< [in] Pointer to the log index file
    uint32_t indexfile_size,            ///< [in] Size of the log index file
    time_t start_ts,                    ///< [in] Starting point of the chosen time period
    time_t end_ts,                      ///< [in] Endpoint of the chosen time period
    const int8_t resolution = -128,     ///< [in] Resolution of timestamps in the log file
    uint32_t file_break = 0             ///< [in] Where was the last datapoint logged (necessary for CircularLog)
) {
    // If file break was not specified (not a circular log type), it should be equal to file_size
    if (!file_break) file_break = file_size;

    // Switch boundaries if they have the wrong order
    if (end_ts < start_ts) std::swap(start_ts, end_ts);

    // Find first timestamp value in the log file
    time_t first_ts = T<E>::find_first_timestamp(file, file_size, file_break);
    // Adjust start_ts if it is out of bounds
    start_ts = start_ts < first_ts ? first_ts : start_ts;

    // Find last timestamp value in the log file
    time_t last_ts = T<E>::find_last_timestamp(file, file_break, resolution);
    // Adjust end_ts if it is out of bounds
    end_ts = end_ts > last_ts ? last_ts : end_ts;

    // If there is no indexfile
    if (indexfile == NULL) {
        uint32_t end_location = T<E>::find_log_entry(file, file_size, sizeof(E), end_ts, file_break, true);
        uint32_t start_location = T<E>::find_log_entry(file, file_size, sizeof(E), start_ts, end_location, false);

        return LogSlice<T,E>(file, start_location, end_location, resolution);
    }

    uint32_t index_entries = indexfile_size / LOG_INDEX_ENTRY_SIZE; // Number of index entries

    uint32_t first_index = 0;
    uint32_t second_index = index_entries - 1;

    entry_location_t start = find_entry_location(file, file_size, file_break, indexfile, indexfile_size, sizeof(E), T<E>::find_log_entry, first_index, second_index, start_ts, false);

    second_index = start.index_in_index + 1;
    time_t second_ts;
    std::memcpy(&second_ts, indexfile + second_index * LOG_INDEX_ENTRY_SIZE, sizeof(time_t));
    if (end_ts < second_ts) {  // If start_ts and end_ts are in the same index entry
        uint32_t search_location;
        std::memcpy(&search_location, indexfile + second_index * LOG_INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
        uint32_t end_location = T<E>::find_log_entry(file, file_size, sizeof(E), end_ts, search_location, true);

        return LogSlice<T,E>(file, start.location_in_file, end_location, resolution);
    }

    entry_location_t end = find_entry_location(file, file_size, file_break, indexfile, indexfile_size, sizeof(E), T<E>::find_log_entry, start.index_in_index, second_index, end_ts, true);

    return LogSlice<T,E>(file, start.location_in_file, end.location_in_file, resolution);
}

/**
 * Return the locations of the entries containing start timestamp and end timestamp in the log file
 * Write the returned log slice into new_file
 *
 * @tparam T the type of log (RegularLog, PeriodicLog, etc)
 * @tparam E the datatype of datapoints held in the log
 */
template <template <class> class T, class E>
LogSlice<T,E> log_slice(
    uint8_t* file,                      ///< [in] Pointer to the log file where we search the entry
    uint32_t file_size,                 ///< [in] Size of the log file
    uint8_t* indexfile,                 ///< [in] Pointer to the log index file
    uint32_t indexfile_size,            ///< [in] Size of the log index file
    time_t start_ts,                    ///< [in] Starting point of the chosen time period
    time_t end_ts,                      ///< [in] Endpoint of the chosen time period
    uint8_t* new_file,                  ///< [in] File where the resulting log slice will be copied into
    const int8_t resolution = -128,     ///< [in] Resolution of timestamps in the log file
    uint32_t file_break = 0             ///< [in] Where was the last datapoint logged (necessary for CircularLog)
) {
    LogSlice<T,E> slice = log_slice<T,E>(file, file_size, indexfile, indexfile_size, start_ts, end_ts, resolution, file_break);
    // Create a new log and copy slice contents into new_file
    Log<T,E> new_log = slice.createLog(new_file);
    // Delete the resulting Log object because we don't need it
    vPortFree(new_log.get_obj());

    return slice;
}

}

#endif
