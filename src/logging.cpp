#include <FreeRTOS.h>
#include "logging.h"

#include "mt25ql_driver.h"
#include "ecbal.h"
#include "echal_rcc.h"
#include "echal_gpio.h"

#ifndef LOGGING_CFG_H
#define PATH_LEN 4 // Length of filepaths
#define FORMATSTRING_LEN 4 // Length of formatstrings
#define DEFAULT_BUFFER_SIZE 120 // Default length for data queue and double buffer
#define INDEX_DENSITY 5 // After how many log entries is an index object created
#define INDEX_BUFFER_SIZE 16 // How much index entries can one index hold
#define INDEX_ENTRY_SIZE (sizeof(time_t) + sizeof(uint32_t)) // Size of one index entry
#endif

// Find closest log entry whose timestamp is less than or equal to given timestamp, starting from address file + search_location
template<class T>
uint32_t find_log_entry_regular(uint8_t* file, time_t timestamp, uint32_t search_location, bool succeeding) {
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

// Find location of a given timestamp in file, using index search
template<class T>
std::pair<uint32_t, uint32_t> find_entry_location(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, uint32_t first_index, uint32_t second_index, time_t timestamp, bool succeeding) {
    uint32_t location;

    // Find timestamp at second_index of indexfile
    time_t second_ts;
    std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));

    while (true) {
        uint32_t middle_index = (first_index + second_index) / 2;
        time_t middle_ts;
        std::memcpy(&middle_ts, indexfile + middle_index * INDEX_ENTRY_SIZE, sizeof(time_t));

        if (first_index == second_index) {
            location = find_log_entry_regular<T>(file, timestamp, file_size, succeeding);
            return std::make_pair(location, middle_index);
        } else if (second_index - first_index == 1) {
            uint32_t search_location = file_size;
            if (timestamp < second_ts) std::memcpy(&search_location, indexfile + second_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
            location = find_log_entry_regular<T>(file, timestamp, search_location, succeeding);
            return std::make_pair(location, middle_index);
        } else if (timestamp == middle_ts) {  // Extremely unlikely case
            if (succeeding) {
                uint32_t search_location;
                std::memcpy(&search_location, indexfile + (middle_index + 1) * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
                location = find_log_entry_regular<T>(file, timestamp, search_location, succeeding);
            } else {
                std::memcpy(&location, indexfile + middle_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
            }
            return std::make_pair(location, middle_index);
        } else if (timestamp > middle_ts) {
            first_index = middle_index;
        } else if (timestamp < middle_ts) {
            second_index = middle_index;
            std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));
        }
    }
}

// TODO: kas slice lugemine peaks toimuma seotult logi objektiga või eraldi? Threading?
template<class T>
LogSlice<T> log_slice(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts, int8_t resolution) {
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
        end_location = find_log_entry_regular<T>(file, end_ts, file_size, true);
        start_location = find_log_entry_regular<T>(file, start_ts, end_location, false);

        return LogSlice<T>(file, start_location, end_location, resolution);
    }

    uint32_t index_entries = indexfile_size / INDEX_ENTRY_SIZE; // Number of index entries

    uint32_t first_index = 0;
    uint32_t second_index = index_entries - 1;

    std::pair<uint32_t, uint32_t> start = find_entry_location<T>(file, file_size, indexfile, indexfile_size, first_index, second_index, start_ts, false);
    start_location = start.first;

    second_index = start.second + 1;
    time_t second_ts;
    std::memcpy(&second_ts, indexfile + second_index * INDEX_ENTRY_SIZE, sizeof(time_t));
    if (end_ts < second_ts) {  // If start_ts and end_ts are in the same index entry
        uint32_t search_location;
        std::memcpy(&search_location, indexfile + second_index * INDEX_ENTRY_SIZE + sizeof(time_t), sizeof(uint32_t));
        end_location = find_log_entry_regular<T>(file, end_ts, search_location, true);

        return LogSlice<T>(file, start_location, end_location, resolution);
    }

    std::pair<uint32_t, uint32_t> end = find_entry_location<T>(file, file_size, indexfile, indexfile_size, start.second, second_index, end_ts, true);
    end_location = end.first;

    return LogSlice<T>(file, start_location, end_location, resolution);
}

/******** BaseLog public functions ********/

template <class T>
uint32_t BaseLog<T>::get_file_size() {
    return this->file_size;
}

template <class T>
void BaseLog<T>::save_meta_info() {
    if (this->metafile == NULL) return;
    this->serialize_meta_info();
}

template <class T>
BaseLog<T>::BaseLog(uint8_t* file)
    : file(file)
    , file_size{0}
    , file_entries{0}
    , metafile(NULL)
    , indexfile(NULL)
    , indexfile_size{0}
    , queue_len{0}
{}

template <class T>
BaseLog<T>::BaseLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file)
    : file(file)
    , metafile(metafile)
    , indexfile(indexfile)
    , queue_len{0}
{
    this->deserialize_meta_info(this->metafile);
}

/******** BaseLog private functions ********/

template <class T>
void BaseLog<T>::write_to_queue(auto var_address, uint8_t len) {
    std::memcpy(this->data_queue + this->queue_len, var_address, len);
    this->queue_len += len;
}

template <class T>
void BaseLog<T>::switch_buffers() {
    // Switch data queue and double buffer
    uint8_t* temp = this->data_queue;
    this->data_queue = this->double_buffer;
    this->double_buffer = temp;

    // Write data to file
    this->write_to_file(this->queue_len);
}

template <class T>
void BaseLog<T>::deserialize_meta_info(uint8_t* metafile) {
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
uint8_t* BaseLog<T>::serialize_meta_info() {
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
void BaseLog<T>::write_to_index(time_t timestamp, uint32_t location) {
    uint8_t* indexfile_pos = this->indexfile + this->indexfile_size;

    std::memcpy(indexfile_pos, &timestamp, sizeof(time_t));
    std::memcpy(indexfile_pos + sizeof(time_t), &location, sizeof(uint32_t));

    this->indexfile_size += INDEX_ENTRY_SIZE;
}

// Dummy
template <class T>
void BaseLog<T>::write_to_file(uint32_t size) {
    std::memcpy(this->file + this->file_size, this->double_buffer, size);
    // A size worth of bytes was appended to the log file
    this->file_size += size;
}

/******** RegularLog public functions ********/

template <class T>
RegularLog<T>::RegularLog(uint8_t* file, int8_t resolution)
    : BaseLog<T>(file)
    , last_timestamp{0}
    , data_added{0}
    , resolution{resolution}
{
    this->data_queue = (uint8_t *)pvPortMalloc(this->min_queue_size());
    this->double_buffer = (uint8_t *)pvPortMalloc(this->min_queue_size());
}

template <class T>
RegularLog<T>::RegularLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file, int8_t resolution)
    : BaseLog<T>(metafile, indexfile, file)
    , last_timestamp{0}
    , data_added{0}
    , resolution{resolution}
{
    this->data_queue = (uint8_t *)pvPortMalloc(this->min_queue_size());
    this->double_buffer = (uint8_t *)pvPortMalloc(this->min_queue_size());
}

template <class T>
void RegularLog<T>::log(T& data, time_t timestamp) {
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
void RegularLog<T>::flush() {
    this->end_entry();
    this->last_timestamp = 0;
}

template <class T>
LogSlice<T> RegularLog<T>::slice(time_t start_ts, time_t end_ts) {
    return log_slice<T>(this->file, this->file_size, this->indexfile, this->indexfile_size, start_ts, end_ts, this->resolution);
}

/******** RegularLog private functions ********/

template <class T>
uint32_t RegularLog<T>::min_queue_size() {
    uint32_t queue_size = 0;

    queue_size += sizeof(time_t); // Size of timestamp
    queue_size += sizeof(uint8_t); // Size of data_added
    queue_size += (1 << (sizeof(uint8_t) * CHAR_BIT)) * (sizeof(T) + sizeof(uint8_t)); // 2^{size_of_uint8_t_in_bits} * size_of_one_datapoint_with_timedelta

    return queue_size;
}

template <class T>
uint16_t RegularLog<T>::scale_timedelta(uint64_t timedelta) {
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
void RegularLog<T>::write_to_queue_timestamp() {
    this->write_to_queue(&(this->last_timestamp), sizeof(time_t));
}

template <class T>
void RegularLog<T>::write_to_queue_datapoint(T& datapoint) {
    this->write_to_queue(&datapoint, sizeof(T));
}

template <class T>
void RegularLog<T>::write_to_queue_timedelta(uint8_t timedelta) {
    this->write_to_queue(&timedelta, sizeof(uint8_t));
}

template <class T>
void RegularLog<T>::write_to_queue_data_added() {
    this->write_to_queue(&(this->data_added), sizeof(uint8_t));
    this->switch_buffers();
}

template <class T>
void RegularLog<T>::end_entry() {
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

/******** LogSlice public functions ********/

template <class T>
LogSlice<T>::LogSlice(uint8_t* file, uint32_t start_location, uint32_t end_location, int8_t resolution)
    : file(file)
    , start_location{start_location}
    , end_location{end_location}
    , resolution{resolution}
{}

template <class T>
uint8_t* LogSlice<T>::get_file() {
    return this->file;
}

template <class T>
uint32_t LogSlice<T>::get_start_location() {
    return this->start_location;
}

template <class T>
uint32_t LogSlice<T>::get_end_location() {
    return this->end_location;
}

template <class T>
int8_t LogSlice<T>::get_resolution() {
    return this->resolution;
}

template <class T>
BaseLog<T> LogSlice<T>::createLog(uint8_t* new_file) {
    std::memcpy(new_file, this->file + this->start_location, this->end_location - this->start_location);
    return RegularLog<T>(new_file, this->resolution);
}

/******** SimpleLog public functions ********/

template <class T>
SimpleLog<T>::SimpleLog(uint8_t* file)
    : BaseLog<T>(file)
    , data_added{0}
    , last_index_ts{0}
    , last_index_pos{0}
{
    this->data_queue = (uint8_t *)pvPortMalloc(DEFAULT_BUFFER_SIZE);
    this->double_buffer = (uint8_t *)pvPortMalloc(DEFAULT_BUFFER_SIZE);
}

template <class T>
SimpleLog<T>::SimpleLog(uint8_t* metafile, uint8_t* indexfile, uint8_t* file)
    : BaseLog<T>(metafile, indexfile, file)
    , data_added{0}
    , last_index_ts{0}
    , last_index_pos{0}
{
    this->data_queue = (uint8_t *)pvPortMalloc(DEFAULT_BUFFER_SIZE);
    this->double_buffer = (uint8_t *)pvPortMalloc(DEFAULT_BUFFER_SIZE);
}

template <class T>
void SimpleLog<T>::log(T& data, time_t timestamp) {
    this->write_to_queue(&timestamp, sizeof(time_t));
    this->write_to_queue(&data, sizeof(T));
    this->data_added++;

    // If enough data has been logged for a new index entry, create it first in volatile memory
    if (this->indexfile != NULL && (this->file_entries + this->data_added) % INDEX_DENSITY == 1) {
        this->last_index_ts = timestamp;
        this->last_index_pos = this->file_size + this->queue_len - sizeof(time_t) - sizeof(T);
    }

    // If there isn't room in the buffer for another entry, write data buffer to log file
    if (DEFAULT_BUFFER_SIZE - this->queue_len < sizeof(time_t) + sizeof(T)) {
        // Write buffer with datapoints to file
        this->switch_buffers();
        this->file_entries += this->data_added;

        // Update the index too (write index entry from volatile memory to file)
        if (this->indexfile != NULL) {
            this->write_to_index(this->last_index_ts, this->last_index_pos);
            this->last_index_ts = 0;
            this->last_index_pos = 0;
        }

        // Reset queue length and data added to queue to 0
        this->queue_len = 0;
        this->data_added = 0;
    }
}

/******** SimpleLog private functions? ********/

template class BaseLog<int>;
template class BaseLog<double>;
template class RegularLog<int>;
template class RegularLog<double>;
template class SimpleLog<int>;
template class SimpleLog<double>;
template class LogSlice<int>;
template class LogSlice<double>;
template LogSlice<int> log_slice<int>(uint8_t* file, uint32_t file_size, uint8_t* indexfile, uint32_t indexfile_size, time_t start_ts, time_t end_ts, int8_t resolution);
