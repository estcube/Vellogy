#include <cstdlib>
#include <cstring>

#include "logging.h"

#include "mt25ql_driver.h"
#include "ecbal.h"
#include "echal_rcc.h"
#include "echal_gpio.h"

template <class T, class U>
uint32_t Log<T,U>::min_queue_size() {
    uint32_t queue_size = 0;

    queue_size += sizeof(time_t); // Size of timestamp
    queue_size += sizeof(U); // Size of data_added
    queue_size += (1 << (sizeof(U) * BYTE_SIZE)) * (sizeof(T) + sizeof(U)); // 2^{size_of_U_in_bits} * size_of_one_datapoint_with_timedelta

    return queue_size;
}

template <class T, class U>
Log<T,U>::Log() {
    this->data_queue = (uint8_t *)malloc(this->min_queue_size());
    this->double_buffer = (uint8_t *)malloc(this->min_queue_size());

    // this->init_metafile();
    // this->init_file();
}

template <class T, class U>
Log<T,U>::Log(uint8_t* metafile, uint8_t* file) {
    this->metafile = metafile;
    this->file = file;
    this->deserialize_meta_info(this->metafile);

    this->data_queue = (uint8_t *)malloc(this->min_queue_size());
    this->double_buffer = (uint8_t *)malloc(this->min_queue_size());
}

template <class T, class U>
void Log<T,U>::log(T* data, time_t timestamp) {
    bool new_entry = false;

    if (!this->last_timestamp || timestamp - this->last_timestamp >= (1 << (sizeof(U) * BYTE_SIZE))) {
        // If one timestamp resolution is full, write to queue how many datapoints were recorded under the previous timestamp
        if (this->last_timestamp) {
            this->write_to_queue_data_added();
            // A new entry was written to the log file
            this->file_entries++;

            // Create an index entry if enough log entries have been added to the log file
            if (this->file_entries % INDEX_DENSITY == 1) {
                this->index_ts[this->file_entries / INDEX_DENSITY] = this->last_timestamp;
                this->index_pos[this->file_entries / INDEX_DENSITY] = this->file_size - this->queue_len;
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
    // this->write_to_file(this->queue_len);

    // A queue_len worth of bytes was appended to the log file
    this->file_size += this->queue_len;
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
    std::memcpy(&(this->index_entries), metafile_pos, sizeof(this->index_entries));
    metafile_pos += sizeof(this->index_entries);

    for (uint32_t i = 0; i < this->index_entries; i++) {
        std::memcpy(this->index_ts + i, metafile_pos, sizeof(time_t));
        std::memcpy(this->index_pos + i, metafile_pos + sizeof(time_t), sizeof(uint32_t));
        metafile_pos += sizeof(time_t) + sizeof(uint32_t);
    }
}

template <class T, class U>
uint8_t* Log<T,U>::serialize_meta_info() {
    uint8_t* metafile_pos = metafile;

    std::memcpy(metafile_pos, &(this->decode_info), sizeof(this->decode_info));
    metafile_pos += sizeof(this->decode_info);
    std::memcpy(metafile_pos, &(this->file_entries), sizeof(this->file_entries));
    metafile_pos += sizeof(this->file_entries);
    std::memcpy(metafile_pos, &(this->file_size), sizeof(this->file_size));
    metafile_pos += sizeof(this->file_size);
    std::memcpy(metafile_pos, &(this->index_entries), sizeof(this->index_entries));
    metafile_pos += sizeof(this->index_entries);

    for (uint32_t i = 0; i < this->index_entries; i++) {
        std::memcpy(metafile_pos, this->index_ts + i, sizeof(time_t));
        std::memcpy(metafile_pos + sizeof(time_t), this->index_pos + i, sizeof(uint32_t));
        metafile_pos += sizeof(time_t) + sizeof(uint32_t);
    }

    return this->metafile;
}

template <class T, class U>
void Log<T,U>::save_meta_info() {
    this->serialize_meta_info();
}

// Dummy
template <class T, class U>
void Log<T,U>::init_metafile() {
    return;
}

// Dummy
template <class T, class U>
void Log<T,U>::init_file() {
    hal_errors_t hal_error = HAL_OK;
    mt25ql_errors_t mt_error = MT25QL_OK;

    hal_rcc_init();

    // Enable flash
    hal_gpio_pin_cfg_t pin_cfg = hal_gpio_pin_cfg_default;
    hal_error = hal_gpio_pin_init(FLASH1_EN, &pin_cfg);
    hal_error = hal_gpio_pin_write(FLASH1_EN, FLASH1_EN_ACTIVE);

    // Wait a little bit
    for(uint32_t i = 100000; i; i--);

    mt25ql_cfg_t cfg_table = {
        .command_mode = HAL_QSPI_QUADSPI_PHASE,   // Commands/data on four lines (per flash)
        .wrap = MT25QL_WRAP_CONTINUOUS
    };

    mt25ql_pins_t pin_table = {
        .clk = FLASH1_CLK,
        .dq = {FLASH1_BK1_IO0, FLASH1_BK1_IO1, FLASH1_BK1_IO2, FLASH1_BK1_IO3,
               FLASH2_BK2_IO0, FLASH2_BK2_IO1, FLASH2_BK2_IO2, FLASH2_BK2_IO3},
        .cs = {FLASH1_BK1_CS, FLASH2_BK2_CS}
    };

    mt_error = mt25ql_initialize(&cfg_table, &pin_table);
    mt_error = mt25ql_unlock_sectors();
}

// Dummy
template <class T, class U>
void Log<T,U>::write_to_file(uint32_t size) {
    mt25ql_errors_t mt_error = MT25QL_OK;
    mt_error = mt25ql_program(this->double_buffer, 0x0000F000, size);

    // Debug purposes
    uint8_t read_queue[4096] = {};
    mt_error = mt25ql_read(read_queue, 0x0000F000, 4096);
}
