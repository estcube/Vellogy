#include <FreeRTOS.h>
#include "logging.h"

#include "mt25ql_driver.h"
#include "ecbal.h"
#include "echal_rcc.h"
#include "echal_gpio.h"

#ifndef LOGGING_CFG_H
#define PATH_LEN 4 // Length of filepaths
#define FORMATSTRING_LEN 4 // Length of formatstrings
#define INDEX_DENSITY 5 // After how many log entries is an index object created
#define INDEX_BUFFER_SIZE 16 // How much index entries can one index hold
#define INDEX_ENTRY_SIZE (sizeof(time_t) + sizeof(uint32_t)) // Size of one index entry
#endif

template<class T>
uint32_t find_entry(uint8_t* file, time_t timestamp, uint32_t search_location, bool succeeding) {
    uint32_t reading_location = search_location;

    bool done = false;
    while (!done) {
        reading_location -= sizeof(uint8_t);
        uint8_t data_added;
        std::memcpy(&data_added, file + reading_location, sizeof(uint8_t));

        // Let's jump ahead and read the timestamp in the beginning of the entry
        uint32_t data_size = (data_added + 1)*(sizeof(T) + sizeof(uint8_t));
        reading_location -= data_size + sizeof(time_t);
        time_t entry_ts;
        std::memcpy(&entry_ts, file + reading_location, sizeof(time_t));

        // The timestamp we are looking for is in this entry
        if (timestamp >= entry_ts) {
            // if we need to find the succeeding entry to the one containing the timestamp instead
            if (succeeding) reading_location += sizeof(time_t) + data_size + sizeof(uint8_t);

            // Exit search loop
            done = true;
        }
    }

    return reading_location;
}

// TODO: kas slice lugemine peaks toimuma seotult logi objektiga või eraldi? Threading?
template<class T>
log_slice_t log_slice(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts) {
    // Return invalid type of log slice when boundaries are not ordered
    if (end_ts < start_ts) std::swap(start_ts, end_ts);

    // Find first timestamp value in the log file
    time_t first_ts;
    std::memcpy(&first_ts, file, sizeof(time_t));
    // Adjust start_ts if it is out of bounds
    start_ts = start_ts < first_ts ? first_ts : start_ts;

    // Find last timestamp value in the log file
    time_t last_ts;
    uint8_t last_data_added;
    std::memcpy(&last_data_added, file + file_size - sizeof(uint8_t), sizeof(uint8_t));
    uint8_t last_timedelta;
    std::memcpy(&last_timedelta, file + file_size - 2 * sizeof(uint8_t), sizeof(uint8_t));
    std::memcpy(&last_ts, file + file_size - sizeof(uint8_t) - (last_data_added + 1)*(sizeof(T) + sizeof(uint8_t)) - sizeof(time_t), sizeof(time_t));
    last_ts += last_timedelta;
    // Adjust end_ts if it is out of bounds
    end_ts = end_ts > last_ts ? last_ts : end_ts;

    uint32_t start_location, end_location;  // Locations of the log entries containing start_ts and end_ts, respectively, end_ts is non-inclusive (in the log file)

    // If there is no indexfile
    if (indexfile == NULL) {
        start_location = find_entry<T>(file, start_ts, file_size, false);
        end_location = find_entry<T>(file, end_ts, file_size, true);

        log_slice_t slice = { .file = file, .start_location = start_location, .end_location = end_location };
        return slice;
    }

    uint32_t index_entries = indexfile_size / INDEX_ENTRY_SIZE; // Number of index entries

    uint32_t first_index = 0;
    uint32_t second_index = index_entries - 1;

    // Find timestamp at second_index of indexfile
    time_t second_ts;
    std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));

    bool done = false;
    while (!done) {
        uint32_t middle_index = (first_index + second_index) / 2;
        time_t middle_ts;
        std::memcpy(&middle_ts, indexfile + middle_index * INDEX_ENTRY_SIZE, sizeof(time_t));

        if (first_index == second_index) {
            start_location = find_entry<T>(file, start_ts, file_size, false);
            first_index = middle_index;  // Actually it already is middle_index, but for clarity
            done = true;
        } else if (second_index - first_index == 1) {
            uint32_t search_location = file_size;
            if (start_ts < second_ts) std::memcpy(&search_location, indexfile + second_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
            start_location = find_entry<T>(file, start_ts, search_location, false);
            first_index = middle_index;  // Actually it already is middle_index, but for clarity
            done = true;
        } else if (start_ts == middle_ts) {  // Extremely unlikely case
            std::memcpy(&start_location, indexfile + middle_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
            first_index = middle_index;  // first_index is now index of start_ts in the indexfile entries list
            done = true;
        } else if (start_ts > middle_ts) {
            first_index = middle_index;
        } else if (start_ts < middle_ts) {
            second_index = middle_index;
            std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));
        }
    }

    done = false;

    if (end_ts < second_ts) {  // If start_ts and end_ts are in the same index entry
        uint32_t search_location;
        std::memcpy(&search_location, indexfile + second_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
        end_location = find_entry<T>(file, end_ts, search_location, true);

        log_slice_t slice = { .file = file, .start_location = start_location, .end_location = end_location };
        return slice;
    }

    // If end_ts is in a different index entry than start_ts
    second_index = index_entries - 1;
    std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));
    while (!done) {
        uint32_t middle_index = (first_index + second_index) / 2;
        time_t middle_ts;
        std::memcpy(&middle_ts, indexfile + middle_index * INDEX_ENTRY_SIZE, sizeof(time_t));

        if (first_index == second_index) {
            end_location = find_entry<T>(file, end_ts, file_size, true);
            done = true;
        } else if (second_index - first_index == 1) {
            uint32_t search_location = file_size;
            if (end_ts < second_ts) std::memcpy(&search_location, indexfile + second_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
            end_location = find_entry<T>(file, end_ts, search_location, true);
            done = true;
        } else if (end_ts == middle_ts) {  // Extremely unlikely case
            uint32_t search_location;
            // end_location is the location of the next timestamp after middle_ts, because it is non-inclusive
            std::memcpy(&search_location, indexfile + (middle_index + 1) * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
            end_location = find_entry<T>(file, end_ts, search_location, true);
            done = true;
        } else if (end_ts > middle_ts) {
            first_index = middle_index;
        } else if (end_ts < middle_ts) {
            second_index = middle_index;
            std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));
        }
    }

    log_slice_t slice = { .file = file, .start_location = start_location, .end_location = end_location };
    return slice;
}

template <class T>
Log<T>::Log(uint8_t* file, int8_t resolution) {
    this->file = file;
    this->resolution = resolution;

    this->data_queue = (uint8_t *)pvPortMalloc(this->min_queue_size());
    this->double_buffer = (uint8_t *)pvPortMalloc(this->min_queue_size());
}

template <class T>
Log<T>::Log(uint8_t* metafile, uint8_t* indexfile, uint8_t* file, int8_t resolution) {
    this->metafile = metafile;
    this->indexfile = indexfile;
    this->file = file;
    this->resolution = resolution;
    this->deserialize_meta_info(this->metafile);

    this->data_queue = (uint8_t *)pvPortMalloc(this->min_queue_size());
    this->double_buffer = (uint8_t *)pvPortMalloc(this->min_queue_size());
}

template <class T>
void Log<T>::log(T& data, time_t timestamp) {
    bool new_entry = false;

    if (!this->last_timestamp
        || this->scale_timedelta(timestamp - this->last_timestamp) >= (1 << (sizeof(uint8_t) * CHAR_BIT))
        || this->data_added >= (1 << (sizeof(uint8_t) * CHAR_BIT)) - 1) {
        // If one timestamp resolution is full, write to queue how many datapoints were recorded under the previous timestamp
        if (this->last_timestamp) {
            this->end_entry();
        }
        // Create a new timestamp and write it to queue
        this->last_timestamp = timestamp;
        this->write_to_queue_timestamp();
        // This is a new entry with a new timestamp
        new_entry = true;
    }

    // Write datapoint to queue
    this->write_to_queue_datapoint(data);
    // If time_diff was bigger than 8 bits, then a new entry was created above, so this cast is safe
    uint8_t time_diff = (uint8_t) this->scale_timedelta(timestamp - this->last_timestamp);
    this->write_to_queue_timedelta(time_diff);

    // The maximum number of datapoints under one timestamp is equal to maximum number of different deltas, 2^(size of uint8_t in bits)
    // To save memory, the number of datapoints written to the end of the entry is one less than there actually are
    // For example, 1 datapoint is written as 0, 2 as 1 and so on
    // This works because an entry with only a timestamp and 0 datapoints is impossible
    // We will not increment data_added on the first datapoint (when delta is zero and new_entry is true)
    if (new_entry) {
        this->data_added = 0;
        new_entry = false;
        return;
    }

    this->data_added++;
}

template <class T>
uint32_t Log<T>::get_file_size() {
    return this->file_size;
}

template <class T>
void Log<T>::save_meta_info() {
    if (this->metafile == NULL) return;
    this->serialize_meta_info();
}

template <class T>
void Log<T>::flush() {
    this->end_entry();
    this->last_timestamp = 0;
}

template <class T>
log_slice_t Log<T>::slice(time_t start_ts, time_t end_ts) {
    return log_slice<T>(this->file, this->file_size, this->indexfile, this->indexfile_size, start_ts, end_ts);
}

/******** Private functions ********/

template <class T>
uint32_t Log<T>::min_queue_size() {
    uint32_t queue_size = 0;

    queue_size += sizeof(time_t); // Size of timestamp
    queue_size += sizeof(uint8_t); // Size of data_added
    queue_size += (1 << (sizeof(uint8_t) * CHAR_BIT)) * (sizeof(T) + sizeof(uint8_t)); // 2^{size_of_uint8_t_in_bits} * size_of_one_datapoint_with_timedelta

    return queue_size;
}

template <class T>
uint16_t Log<T>::scale_timedelta(uint64_t timedelta) {
    // Relative precision: how much less precise is the timedelta we can log compared to the actual timestamp resolution
    // For example, if TS_RESOLUTION is -4 and this->resolution is -2 and timestamp subseconds are in decimal system,
    // then timedeltas actually saved are 100 times less precise than the timestamp type allows
    uint32_t rel_precision = pow(TS_BASE, abs(this->resolution - TS_RESOLUTION));

    // For example, if timedelta is 7863 and rel_precision is 100, then lower_bound is 7800 and upper_bound is 7900
    uint16_t lower_bound = timedelta / rel_precision * rel_precision;
    uint16_t upper_bound = (timedelta / rel_precision + 1) * rel_precision;

    return (upper_bound - timedelta < timedelta - lower_bound ? upper_bound : lower_bound) / rel_precision;
}

template <class T>
void Log<T>::write_to_queue(auto var_address, uint8_t len) {
    std::memcpy(this->data_queue + this->queue_len, var_address, len);
    this->queue_len += len;
}

template <class T>
void Log<T>::write_to_queue_timestamp() {
    this->write_to_queue(&(this->last_timestamp), sizeof(time_t));
}

template <class T>
void Log<T>::write_to_queue_datapoint(T& datapoint) {
    this->write_to_queue(&datapoint, sizeof(T));
}

template <class T>
void Log<T>::write_to_queue_timedelta(uint8_t timedelta) {
    this->write_to_queue(&timedelta, sizeof(uint8_t));
}

template <class T>
void Log<T>::write_to_queue_data_added() {
    this->write_to_queue(&(this->data_added), sizeof(uint8_t));
    this->switch_buffers();
}

template <class T>
void Log<T>::switch_buffers() {
    // Switch data queue and double buffer
    uint8_t* temp = this->data_queue;
    this->data_queue = this->double_buffer;
    this->double_buffer = temp;

    // Write data to file
    this->write_to_file(this->queue_len);
}

template <class T>
void Log<T>::end_entry() {
    this->write_to_queue_data_added();
    // A new entry was written to the log file
    this->file_entries++;

    // Create an index entry (write it to indexfile) if enough log entries have been added to the log file
    if (this->indexfile != NULL && this->file_entries % INDEX_DENSITY == 1) {
        this->write_to_index(this->last_timestamp, this->file_size - this->queue_len);
    }

    // Reset queue length to 0
    this->queue_len = 0;
}

template <class T>
void Log<T>::deserialize_meta_info(uint8_t* metafile) {
    uint8_t* metafile_pos = metafile;

    std::memcpy(&(this->decode_info), metafile_pos, sizeof(this->decode_info));
    metafile_pos += sizeof(this->decode_info);
    std::memcpy(&(this->file_entries), metafile_pos, sizeof(this->file_entries));
    metafile_pos += sizeof(this->file_entries);
    std::memcpy(&(this->file_size), metafile_pos, sizeof(this->file_size));
    metafile_pos += sizeof(this->file_size);
    std::memcpy(&(this->indexfile_size), metafile_pos, sizeof(this->indexfile_size));
    metafile_pos += sizeof(this->indexfile_size);
}

template <class T>
uint8_t* Log<T>::serialize_meta_info() {
    uint8_t* metafile_pos = this->metafile;

    std::memcpy(metafile_pos, &(this->decode_info), sizeof(this->decode_info));
    metafile_pos += sizeof(this->decode_info);
    std::memcpy(metafile_pos, &(this->file_entries), sizeof(this->file_entries));
    metafile_pos += sizeof(this->file_entries);
    std::memcpy(metafile_pos, &(this->file_size), sizeof(this->file_size));
    metafile_pos += sizeof(this->file_size);
    std::memcpy(metafile_pos, &(this->indexfile_size), sizeof(this->indexfile_size));
    metafile_pos += sizeof(this->indexfile_size);

    return this->metafile;
}

// Dummy
template <class T>
void Log<T>::write_to_index(time_t timestamp, uint32_t location) {
    uint8_t* indexfile_pos = this->indexfile + this->indexfile_size;

    std::memcpy(indexfile_pos, &timestamp, sizeof(time_t));
    std::memcpy(indexfile_pos + sizeof(time_t), &location, sizeof(uint32_t));

    this->indexfile_size += INDEX_ENTRY_SIZE;
}

// Dummy
template <class T>
void Log<T>::write_to_file(uint32_t size) {
    std::memcpy(file + file_size, this->double_buffer, size);
    // A size worth of bytes was appended to the log file
    this->file_size += size;
}

template class Log<int>;
template class Log<double>;
template uint32_t find_entry<int>(uint8_t* file, time_t timestamp, uint32_t search_location, bool succeeding);
template log_slice_t log_slice<int>(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts);
