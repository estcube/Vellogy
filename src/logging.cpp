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

    this->init_file();
}

template <class T, class U>
void Log<T,U>::log(T* data, time_t timestamp) {
    bool new_entry = false;

    if (!this->last_timestamp || this->flushed || timestamp - this->last_timestamp >= (1 << (sizeof(U) * BYTE_SIZE))) {
        // If one timestamp resolution is full, write to queue how many datapoints were recorded under the previous timestamp
        if (this->last_timestamp && !(this->flushed)) {
            write_to_queue(&(this->data_added), sizeof(U));
        }
        // Create a new timestamp and write it to queue
        this->last_timestamp = timestamp;
        write_to_queue(&(this->last_timestamp), sizeof(this->last_timestamp));
        this->data_added = 0;

        // This is a new entry with a new timestamp
        new_entry = true;

        // Log is no longer in flushed state
        this->flushed = false;
    }

    // Write datapoint to queue
    this->write_to_queue(data, sizeof(T));
    // TODO: change data type
    uint32_t time_diff = timestamp - this->last_timestamp;
    this->write_to_queue(&time_diff, sizeof(U));

    // The maximum number of datapoints under one timestamp is equal to maximum number of different deltas, 2^(size of U in bits)
    // To save memory, the number of datapoints written to the end of the entry is one less than there actually are
    // For example, 1 datapoint is written as 0, 2 as 1 and so on
    // This works because an entry with only a timestamp and 0 datapoints is impossible
    // We will not increment data_added on the first datapoint (when delta is zero and new_entry is true)
    if (new_entry) {
        new_entry = false;
        return;
    }

    this->data_added++;
}

template <class T, class U>
void Log<T,U>::write_to_queue(auto var_address, uint8_t len) {
    // How many bytes of room is there left in the data queue
    uint32_t remaining_bytes = QUEUE_SIZE - this->queue_len;

    // If a buffer switch is needed halfway through the write
    if (len > remaining_bytes) {
        // Write as many bytes as possible to the current buffer
        memcpy(this->data_queue + this->queue_len, var_address, remaining_bytes);
        this->queue_len += remaining_bytes;

        // Switch buffers
        this->switch_buffers();

        // Write remaining bytes to the new (switched) buffer
        memcpy(this->data_queue, (uint8_t *)var_address + remaining_bytes, len - remaining_bytes);
        this->queue_len += len - remaining_bytes;
    } else { // If all bytes can be written to current buffer
        memcpy(this->data_queue + this->queue_len, var_address, len);
        this->queue_len += len;
    }
}

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

template <class T, class U>
void Log<T,U>::write_to_file(uint32_t size) {
    mt25ql_errors_t mt_error = MT25QL_OK;
    mt_error = mt25ql_program(this->double_buffer, 0x0000F000, size);

    // Debug purposes
    uint8_t read_queue[4096] = {};
    mt_error = mt25ql_read(read_queue, 0x0000F000, 4096);
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
    this->write_to_queue(&(this->data_added), sizeof(U));
    this->flushed = true;

    this->switch_buffers();
}
