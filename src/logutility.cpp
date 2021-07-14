#include "logutility.h"

#ifndef LOGGING_CFG_H
#define PATH_LEN 4 // Length of filepaths
#define FORMATSTRING_LEN 4 // Length of formatstrings
#define INDEX_DENSITY 5 // After how many log entries is an index object created
#define INDEX_BUFFER_SIZE 16 // How much index entries can one index hold
#define INDEX_ENTRY_SIZE (sizeof(time_t) + sizeof(uint32_t)) // Size of one index entry
#endif

struct entry_location_t {
    uint32_t location_in_file; // Location of log entry in file
    uint32_t index_in_index; // Which index entry contains the log entry
};

// Find location of a given timestamp in file, using index search
template <template <class> class T, class E>
entry_location_t find_entry_location(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, uint8_t datapoint_size,
    uint32_t first_index, uint32_t second_index, time_t timestamp, bool succeeding) {

    uint32_t location;

    // If no binary search is needed
    // TODO: wtf
    if (first_index == second_index) {
        location = T<E>::find_log_entry(file, datapoint_size, timestamp, file_size, succeeding);
        return (entry_location_t) { .location_in_file = location, .index_in_index = first_index };
    }

    // Find timestamp at second_index of indexfile
    time_t second_ts;
    std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));

    while (true) {
        uint32_t middle_index = (first_index + second_index) / 2;
        time_t middle_ts;
        std::memcpy(&middle_ts, indexfile + middle_index * INDEX_ENTRY_SIZE, sizeof(time_t));
        bool ts_exact_match = timestamp == middle_ts;  // Timestamp happened to be an index entry timestamp (quite unlikely)

        if (second_index - first_index == 1 || ts_exact_match) {  // The binary search is complete
            uint32_t search_index = ts_exact_match ? (succeeding ? middle_index + 1 : middle_index) : (timestamp < second_ts ? second_index : UINT_MAX);

            // If timestamp was greater than or equal to second_ts, start looking for it from the end of the file
            // Otherwise start searching from the location that is saved in indexfile under entry numbered search_index
            uint32_t search_location = file_size;
            if (search_index != UINT_MAX) std::memcpy(&search_location, indexfile + search_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));

            // If the timestamp happened to be an exact index entry timestamp, the search_location already is the location of the timestamp in the log file
            location = ts_exact_match && !succeeding ? search_location : T<E>::find_log_entry(file, datapoint_size, timestamp, search_location, succeeding);
            return (entry_location_t) { .location_in_file = location, .index_in_index = middle_index };
        } else if (timestamp > middle_ts) {
            first_index = middle_index;
        } else if (timestamp < middle_ts) {
            second_index = middle_index;
            std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));
        }
    }
}

template <template <class> class T, class E>
LogSlice<T,E> log_slice(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts, int8_t resolution) {
    // Switch boundaries if they have the wrong order
    if (end_ts < start_ts) std::swap(start_ts, end_ts);

    // Find first timestamp value in the log file
    time_t first_ts;
    std::memcpy(&first_ts, file, sizeof(time_t));
    // Adjust start_ts if it is out of bounds
    start_ts = start_ts < first_ts ? first_ts : start_ts;

    // Find last timestamp value in the log file
    time_t last_ts = T<E>::find_last_timestamp(file, file_size, resolution);
    // Adjust end_ts if it is out of bounds
    end_ts = end_ts > last_ts ? last_ts : end_ts;

    // If there is no indexfile
    if (indexfile == NULL) {
        uint32_t end_location = T<E>::find_log_entry(file, sizeof(E), end_ts, file_size, true);
        uint32_t start_location = T<E>::find_log_entry(file, sizeof(E), start_ts, end_location, false);

        return LogSlice<T,E>(file, start_location, end_location, resolution);
    }

    uint32_t index_entries = indexfile_size / INDEX_ENTRY_SIZE; // Number of index entries

    uint32_t first_index = 0;
    uint32_t second_index = index_entries - 1;

    entry_location_t start = find_entry_location<T,E>(file, file_size, indexfile, indexfile_size, sizeof(E), first_index, second_index, start_ts, false);

    second_index = start.index_in_index + 1;
    time_t second_ts;
    std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));
    if (end_ts < second_ts) {  // If start_ts and end_ts are in the same index entry
        uint32_t search_location;
        std::memcpy(&search_location, indexfile + second_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
        uint32_t end_location = T<E>::find_log_entry(file, sizeof(E), end_ts, search_location, true);

        return LogSlice<T,E>(file, start.location_in_file, end_location, resolution);
    }

    entry_location_t end = find_entry_location<T,E>(file, file_size, indexfile, indexfile_size, sizeof(E), start.index_in_index, second_index, end_ts, true);

    return LogSlice<T,E>(file, start.location_in_file, end.location_in_file, resolution);
}

template LogSlice<SimpleLog,int> log_slice<SimpleLog, int>(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts, int8_t resolution);
template LogSlice<RegularLog,int> log_slice<RegularLog, int>(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts, int8_t resolution);
