#include "logging.h"

#include "mt25ql_driver.h"
#include "ecbal.h"
#include "echal_rcc.h"
#include "echal_gpio.h"

template<class T, class U>
uint32_t find_entry(uint8_t* file, time_t timestamp, uint32_t search_location, bool succeeding) {
    uint32_t reading_location = search_location;

    bool done = false;
    while (!done) {
        reading_location -= sizeof(U);
        U data_added;
        std::memcpy(&data_added, file + reading_location, sizeof(U));

        // Let's jump ahead and read the timestamp in the beginning of the entry
        uint32_t data_size = (data_added + 1)*(sizeof(T) + sizeof(U));
        reading_location -= data_size + sizeof(time_t);
        time_t entry_ts;
        std::memcpy(&entry_ts, file + reading_location, sizeof(time_t));

        // The timestamp we are looking for is in this entry
        if (timestamp >= entry_ts) {
            // if we need to find the succeeding entry to the one containing the timestamp instead
            if (succeeding) reading_location += sizeof(time_t) + data_size + sizeof(U);

            // Exit search loop
            done = true;
        }
    }

    return reading_location;
}

// TODO: kas slice lugemine peaks toimuma seotult logi objektiga või eraldi? Threading?
template<class T, class U>
log_slice_t log_slice(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts) {
    // Return invalid type of log slice when boundaries are not ordered
    if (end_ts < start_ts) return log_slice_t();

    // Find first timestamp value in the log file
    time_t first_ts;
    std::memcpy(&first_ts, file, sizeof(time_t));
    // Adjust start_ts if it is out of bounds
    start_ts = start_ts < first_ts ? first_ts : start_ts;

    // Find last timestamp value in the log file
    time_t last_ts;
    U last_data_added;
    std::memcpy(&last_data_added, file + file_size - sizeof(U), sizeof(U));
    U last_timedelta;
    std::memcpy(&last_timedelta, file + file_size - 2 * sizeof(U), sizeof(U));
    std::memcpy(&last_ts, file + file_size - sizeof(U) - (last_data_added + 1)*(sizeof(T) + sizeof(U)) - sizeof(time_t), sizeof(time_t));
    last_ts += last_timedelta;
    // Adjust end_ts if it is out of bounds
    end_ts = end_ts > last_ts ? last_ts : end_ts;

    uint32_t start_location, end_location;  // Locations of the log entries containing start_ts and end_ts, respectively, end_ts is non-inclusive (in the log file)

    // If there is no indexfile
    if (indexfile == NULL) {
        start_location = find_entry<T,U>(file, start_ts, file_size, false);
        end_location = find_entry<T,U>(file, end_ts, file_size, true);

        log_slice_t slice;
        slice.start_location = start_location;
        slice.end_location = end_location;
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
            start_location = find_entry<T,U>(file, start_ts, file_size, false);
            first_index = middle_index;  // Actually it already is middle_index, but for clarity
            done = true;
        } else if (second_index - first_index == 1) {
            uint32_t search_location = file_size;
            if (start_ts < second_ts) std::memcpy(&search_location, indexfile + second_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
            start_location = find_entry<T,U>(file, start_ts, search_location, false);
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
        end_location = find_entry<T,U>(file, end_ts, search_location, true);

        log_slice_t slice;
        slice.start_location = start_location;
        slice.end_location = end_location;
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
            end_location = find_entry<T,U>(file, end_ts, file_size, true);
            done = true;
        } else if (second_index - first_index == 1) {
            uint32_t search_location = file_size;
            if (end_ts < second_ts) std::memcpy(&search_location, indexfile + second_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
            end_location = find_entry<T,U>(file, end_ts, search_location, true);
            done = true;
        } else if (end_ts == middle_ts) {  // Extremely unlikely case
            uint32_t search_location;
            // end_location is the location of the next timestamp after middle_ts, because it is non-inclusive
            std::memcpy(&search_location, indexfile + (middle_index + 1) * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
            end_location = find_entry<T,U>(file, end_ts, search_location, true);
            done = true;
        } else if (end_ts > middle_ts) {
            first_index = middle_index;
        } else if (end_ts < middle_ts) {
            second_index = middle_index;
            std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));
        }
    }

    log_slice_t slice;
    slice.start_location = start_location;
    slice.end_location = end_location;
    return slice;
}

template <class T, class U>
uint32_t Log<T,U>::min_queue_size() {
    uint32_t queue_size = 0;

    queue_size += sizeof(time_t); // Size of timestamp
    queue_size += sizeof(U); // Size of data_added
    queue_size += (1 << (sizeof(U) * BYTE_SIZE)) * (sizeof(T) + sizeof(U)); // 2^{size_of_U_in_bits} * size_of_one_datapoint_with_timedelta

    return queue_size;
}

template <class T, class U>
Log<T,U>::Log(bool has_metafile) {
    this->data_queue = (uint8_t *)malloc(this->min_queue_size());
    this->double_buffer = (uint8_t *)malloc(this->min_queue_size());

    if (has_metafile) {
        this->init_metafile();
        this->init_indexfile();
    }
    this->init_file();
}

template <class T, class U>
Log<T,U>::Log(uint8_t* file, bool has_metafile) {
    this->file = file;

    this->data_queue = (uint8_t *)malloc(this->min_queue_size());
    this->double_buffer = (uint8_t *)malloc(this->min_queue_size());

    if (has_metafile) {
        this->init_metafile();
        this->init_indexfile();
    }
}

template <class T, class U>
Log<T,U>::Log(uint8_t* metafile, uint8_t* indexfile, uint8_t* file) {
    this->metafile = metafile;
    this->indexfile = indexfile;
    this->file = file;
    this->deserialize_meta_info(this->metafile);

    this->data_queue = (uint8_t *)malloc(this->min_queue_size());
    this->double_buffer = (uint8_t *)malloc(this->min_queue_size());
}

template <class T, class U>
void Log<T,U>::log(T* data, time_t timestamp) {
    bool new_entry = false;

    if (!this->last_timestamp || timestamp - this->last_timestamp >= (1 << (sizeof(U) * BYTE_SIZE)) || this->data_added >= (1 << (sizeof(U) * BYTE_SIZE)) - 1) {
        // If one timestamp resolution is full, write to queue how many datapoints were recorded under the previous timestamp
        if (this->last_timestamp) {
            this->write_to_queue_data_added();
            // A new entry was written to the log file
            this->file_entries++;

            // Create an index entry if enough log entries have been added to the log file
            if (this->indexfile != NULL && this->file_entries % INDEX_DENSITY == 1) {
                this->index_ts[this->index_entries] = this->last_timestamp;
                this->index_pos[this->index_entries] = this->file_size - this->queue_len;
                this->index_entries++;
            }

            // Reset queue length to 0
            this->queue_len = 0;
        }
        // Create a new timestamp and write it to queue
        this->last_timestamp = timestamp;
        this->write_to_queue_timestamp();
        // This is a new entry with a new timestamp
        new_entry = true;
    }

    // Write datapoint to queue
    this->write_to_queue_datapoint(data);
    U time_diff = timestamp - this->last_timestamp;
    this->write_to_queue_timedelta(time_diff);

    // The maximum number of datapoints under one timestamp is equal to maximum number of different deltas, 2^(size of U in bits)
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

template <class T, class U>
void Log<T,U>::write_to_queue_timestamp() {
    this->write_to_queue(&(this->last_timestamp), sizeof(time_t));
}

template <class T, class U>
void Log<T,U>::write_to_queue_datapoint(T* datapoint) {
    this->write_to_queue(datapoint, sizeof(T));
}

template <class T, class U>
void Log<T,U>::write_to_queue_timedelta(U timedelta) {
    this->write_to_queue(&timedelta, sizeof(U));
}

template <class T, class U>
void Log<T,U>::write_to_queue_data_added() {
    this->write_to_queue(&(this->data_added), sizeof(U));
    this->switch_buffers();
}


template <class T, class U>
void Log<T,U>::write_to_queue(auto var_address, uint8_t len) {
    std::memcpy(this->data_queue + this->queue_len, var_address, len);
    this->queue_len += len;
}

template <class T, class U>
void Log<T,U>::switch_buffers() {
    // Switch data queue and double buffer
    uint8_t* temp = this->data_queue;
    this->data_queue = this->double_buffer;
    this->double_buffer = temp;

    // Write data to file
    this->write_to_file(this->queue_len);
}

template <class T, class U>
void Log<T,U>::deserialize_meta_info(uint8_t* metafile) {
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

template <class T, class U>
uint8_t* Log<T,U>::serialize_meta_info() {
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

template <class T, class U>
uint8_t* Log<T,U>::serialize_index() {
    uint8_t* indexfile_pos = this->indexfile + this->indexfile_size;

    for (uint32_t i = 0; i < this->index_entries; i++) {
        std::memcpy(indexfile_pos, this->index_ts + i, sizeof(time_t));
        std::memcpy(indexfile_pos + sizeof(time_t), this->index_pos + i, sizeof(uint32_t));
        indexfile_pos += INDEX_ENTRY_SIZE;
    }

    this->indexfile_size = indexfile_pos - this->indexfile;
    this->index_entries = 0;

    return this->indexfile;
}

template <class T, class U>
uint32_t Log<T,U>::get_file_size() {
    return this->file_size;
}

template <class T, class U>
void Log<T,U>::save_meta_info() {
    if (this->metafile == NULL) return;

    this->serialize_index();
    this->serialize_meta_info();
}

// Dummy
template <class T, class U>
void Log<T,U>::init_metafile() {
    return;
}

// Dummy
template <class T, class U>
void Log<T,U>::init_indexfile() {
    return;
}

// Dummy
template <class T, class U>
void Log<T,U>::init_file() {
    return;
}

// Dummy
template <class T, class U>
void Log<T,U>::write_to_file(uint32_t size) {
    std::memcpy(file + file_size, this->double_buffer, size);
    // A size worth of bytes was appended to the log file
    this->file_size += size;
}

template class Log<int, uint8_t>;
// template class Log<int, uint16_t>;
// template class Log<int, uint32_t>;
template class Log<double, uint8_t>;
// template class Log<double, uint16_t>;
// template class Log<double, uint32_t>;
template uint32_t find_entry<int, uint8_t>(uint8_t* file, time_t timestamp, uint32_t search_location, bool succeeding);
template log_slice_t log_slice<int, uint8_t>(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts);
