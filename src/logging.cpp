#include <cstdlib>
#include <cstring>

#include "logging.h"

#include "mt25ql_driver.h"
#include "ecbal.h"
#include "echal_rcc.h"
#include "echal_gpio.h"

template <class T, class U>
Log<T,U>::Log() {
    this->data_queue = (uint8_t *)malloc(QUEUE_SIZE);
    this->double_buffer = (uint8_t *)malloc(QUEUE_SIZE);

    // this->init_metafile();
    // this->init_file();
}

template <class T, class U>
Log<T,U>::Log(uint8_t* metafile, uint8_t* file) {
    this->data_queue = (uint8_t *)malloc(QUEUE_SIZE);
    this->double_buffer = (uint8_t *)malloc(QUEUE_SIZE);

    this->metafile = metafile;
    this->file = file;
    this->deserialize_meta_info(this->metafile);
}

template <class T, class U>
void Log<T,U>::log(T* data, time_t timestamp) {
    bool new_entry = false;

    if (!this->last_timestamp || this->flushed || timestamp - this->last_timestamp >= (1 << (sizeof(U) * BYTE_SIZE))) {
        // If one timestamp resolution is full, write to queue how many datapoints were recorded under the previous timestamp
        if (this->last_timestamp && !(this->flushed)) {
            write_to_queue(&(this->data_added), sizeof(U), DATA_ADDED);
        }
        // Create a new timestamp and write it to queue
        this->last_timestamp = timestamp;
        write_to_queue(&(this->last_timestamp), sizeof(this->last_timestamp), TIMESTAMP);

        // This is a new entry with a new timestamp
        new_entry = true;

        // Log is no longer in flushed state
        this->flushed = false;
    }

    // Write datapoint to queue
    this->write_to_queue(data, sizeof(T), DATAPOINT);
    // TODO: change data type
    uint32_t time_diff = timestamp - this->last_timestamp;
    this->write_to_queue(&time_diff, sizeof(U), TIMEDELTA);

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
void Log<T,U>::write_to_queue(auto var_address, uint8_t len, log_element_t element_type) {
    // How many bytes of room is there left in the data queue
    uint32_t remaining_bytes = QUEUE_SIZE - this->queue_len;

    // If a buffer switch is needed halfway through the write
    if (len > remaining_bytes) {
        // Write as many bytes as possible to the current buffer
        memcpy(this->data_queue + this->queue_len, var_address, remaining_bytes);
        this->queue_len += remaining_bytes;

        // Switch buffers
        this->switch_buffers();

        // There are this->data_added full datapoints now in file (the one currently written might not be legitimate)
        this->data_added_file = this->data_added;
        // To start decoding the file, we need the position of the last timedelta (previously the position of the last timedelta in the buffer)
        this->decode_start_position_file = this->decode_start_position_buffer;

        // Write remaining bytes to the new (switched) buffer
        memcpy(this->data_queue, (uint8_t *)var_address + remaining_bytes, len - remaining_bytes);
        this->queue_len += len - remaining_bytes;
    } else { // If all bytes can be written to current buffer
        memcpy(this->data_queue + this->queue_len, var_address, len);
        this->queue_len += len;

        // To be decoded properly, the last legitimate element needs to be a timedelta (data_added in case of flushed log)
        if (element_type == TIMEDELTA) {
            this->decode_start_position_buffer = this->queue_len;
        }
    }
}

template <class T, class U>
void Log<T,U>::switch_buffers() {
    // Switch data queue and double buffer
    uint8_t* temp = this->data_queue;
    this->data_queue = this->double_buffer;
    this->double_buffer = temp;

    // Write data to file
    // this->write_to_file(this->queue_len);

    // Reset queue length to 0
    this->queue_len = 0;
}

template <class T, class U>
void Log<T,U>::flush_buffer() {
    this->write_to_queue(&(this->data_added), sizeof(U), DATA_ADDED);
    this->flushed = true;

    this->switch_buffers();
}

template <class T, class U>
void Log<T,U>::deserialize_meta_info(uint8_t* metafile) {
    // Dummy structs to see the size of their fields' data types
    log_decode_info_t dummy_decode_info = log_decode_info_t();
    log_internal_state_t dummy_state_info = log_internal_state_t();

    // If space taken up by decode info in the log metafile has not been calculated yet
    if (this->decode_info_offset == 0) {
        this->decode_info_offset = sizeof(dummy_decode_info.type)
        + sizeof(dummy_decode_info.TS_seconds_size)
        + sizeof(dummy_decode_info.TS_subseconds_size)
        + sizeof(dummy_decode_info.U_size);

        // Find the space taken up by the T data type format string
        uint8_t T_formatstring_len = *(this->metafile + this->decode_info_offset);
        this->decode_info_offset += 1 + T_formatstring_len;

        this->decode_info_offset += sizeof(dummy_decode_info.file_size);

        // Find the space taken up by the path to log file
        uint8_t path_len = *(this->metafile + this->decode_info_offset);
        this->decode_info_offset += 1 + path_len;

        this->decode_info_offset += sizeof(dummy_decode_info.buffer_size);
    }

    // Set an appropriate offset to start reading the metafile from internal state info (and skip the decode info)
    uint32_t offset = this->decode_info_offset;

    // Copy info from metafile to Log object fields
    memcpy(&(this->last_timestamp), metafile + offset, sizeof(dummy_state_info.last_timestamp));
    offset += sizeof(dummy_state_info.last_timestamp);
    memcpy(&(this->data_added_file), metafile + offset, sizeof(this->data_added_file));
    offset += sizeof(dummy_state_info.data_added_file);
    this->data_added = this->data_added_file;
    memcpy(&(this->decode_start_position_file), metafile + offset, sizeof(this->decode_start_position_file));
    offset += sizeof(dummy_state_info.decode_start_position_file);

    this->flushed = *((bool*)(metafile + offset));
    offset += sizeof(dummy_state_info.flushed);
}

template <class T, class U>
uint8_t* Log<T,U>::serialize_meta_info() {
    // We only want to change internal state info, not decode info
    uint32_t offset = this->decode_info_offset;

    // Copy internal state info from Log object to metafile
    memcpy(this->metafile + offset, &(this->last_timestamp), sizeof(this->last_timestamp));
    offset += sizeof(this->last_timestamp);
    memcpy(this->metafile + offset, &(this->data_added_file), sizeof(this->data_added_file));
    offset += sizeof(this->data_added_file);

    // data_added_file possibly takes up more bytes than data_added field
    for (uint8_t i = 0; i < sizeof(log_internal_state_t().data_added_file) - sizeof(U); i++) {
        this->metafile[offset] = 0;
        offset++;
    }

    memcpy(this->metafile + offset, &(this->decode_start_position_file), sizeof(this->decode_start_position_file));
    offset += sizeof(this->decode_start_position_file);
    this->metafile[offset] = this->flushed;
    offset++;

    return this->metafile; // return metafile pointer
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
