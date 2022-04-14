/**
 * @file circularlog.hpp
 * Author: Annika Jaakson
 * Refactored by: Kerdo Kurs
 */

#ifndef CIRCULARLOG_H
#define CIRCULARLOG_H

#include "simplelog.hpp"

namespace eclog {
template<class T>
class circular_log : public simple_log<T> {
  private:
  uint32_t file_capacity;
  uint32_t file_break;

  /**
   * Write bytes from active data buffer to log file
   * Dummy
   */
  void write_to_file(uint32_t size ///< [in] How many bytes to write form data queue to log file
  ) {
    // Write as many bytes as possible to the end of the file (might be 0 as well)
    uint32_t first_bytes = size <= this->file_capacity - this->file_break ? size : this->file_capacity -
      this->file_size;

    std::memcpy(this->file + this->file_break, this->double_buffer, first_bytes);
    // Write all the remaining bytes (might be 0 as well) to the beginning of the file
    uint32_t second_bytes = first_bytes == size ? 0 : size - (this->file_capacity - this->file_break);
    std::memcpy(this->file + LOG_MANDATORY_DECODE_INFO_SIZE, this->double_buffer, second_bytes);

    // If all bytes could be written to the end of the file, increment file break by the number of bytes written
    // If not, then new file break is the number of bytes that had to be written in the beginning of the file
    this->file_break = second_bytes ? LOG_MANDATORY_DECODE_INFO_SIZE + second_bytes : this->file_break + size;
    // Until the log file hasn't been filled a single time, file size and file break are the same thing
    // After the log file has been filled once, file size is always equal to the maximal capacity of the file
    this->file_size = this->file_size == this->file_capacity || second_bytes ? this->file_capacity : this->file_break;
  }

  /**
   * Switch data and double buffer
   */
  void switch_buffers() {
    // Switch data queue and double buffer
    uint8_t *temp = this->data_queue;

    this->data_queue = this->double_buffer;
    this->double_buffer = temp;

    // Write data to file
    this->write_to_file(this->queue_len);
  }

  public:
  /**** Constructors ****/

  /**
   * Initialize the log with the given file (no meta- and indexfile)
   */
  circular_log(uint8_t *file, ///< [in] Pointer to the file where datapoints will be saved
    uint32_t file_capacity    ///< [in] Maximum capacity of the log file
  ) : simple_log<T>(file) {
    // If the file capacity provided is not divisible by the size of the log entry, reduce it until it is
    uint8_t excessive_space = (file_capacity - LOG_MANDATORY_DECODE_INFO_SIZE) % (sizeof(time_t) + sizeof(T));

    this->file_capacity = file_capacity - excessive_space;

    // If this is a brand new file without previous datapoints, write the mandatory decode info into the beginning of
    // the file
    if (this->file_size == LOG_MANDATORY_DECODE_INFO_SIZE) {
      this->file_size = 0;

      log_file_type_t file_type = LOG_CIRCULAR;
      log_data_type_t data_type = LOG_INT32_T; // TODO: non-dummy value

      std::memcpy(this->file + this->file_size, &file_type, sizeof(log_file_type_t));
      this->file_size += sizeof(log_file_type_t);
      std::memcpy(this->file + this->file_size, &data_type, sizeof(log_data_type_t));
      this->file_size += sizeof(log_data_type_t);
    }

    // The border of valid info is the same as file size at first
    this->file_break = this->file_size;
  }

  /**
   * Initialize the log (from a metafile) held in the file given
   */
  circular_log(uint8_t *metafile, ///< [in] Pointer to the file where metainfo will be saved
    uint8_t *indexfile,           ///< [in] Pointer to the file where index entries will be saved
    uint8_t *file,                ///< [in] Pointer to the file where datapoints will be saved
    uint32_t file_capacity        ///< [in] Maximum capacity of the log file
  ) : simple_log<T>(metafile, indexfile, file) {
    // If the file capacity provided is not divisible by the size of the log entry, reduce it until it is
    uint8_t excessive_space = (file_capacity - LOG_MANDATORY_DECODE_INFO_SIZE) % (sizeof(time_t) + sizeof(T));

    this->file_capacity = file_capacity - excessive_space;

    // If this is a brand new file without previous datapoints, write the mandatory decode info into the beginning of
    // the file
    if (this->file_size == LOG_MANDATORY_DECODE_INFO_SIZE) {
      this->file_size = 0;

      log_file_type_t file_type = LOG_CIRCULAR;
      log_data_type_t data_type = LOG_INT32_T; // TODO: non-dummy value

      std::memcpy(this->file + this->file_size, &file_type, sizeof(log_file_type_t));
      this->file_size += sizeof(log_file_type_t);
      std::memcpy(this->file + this->file_size, &data_type, sizeof(log_data_type_t));
      this->file_size += sizeof(log_data_type_t);
    }

    // The border of valid info is the same as file size at first
    this->file_break = this->file_size;
  }

  /**** Main functionality ****/

  /**
   * Log data, attach timestamp in function
   */
  void log(
    T &data                      ///< [in] Datapoint to be logged
  );

  /**
   * Log data with a given timestamp in the file
   */
  void log(T &data,  ///< [in] Datapoint to be logged
    time_t timestamp ///< [in] Capturing time of the datapoint
  ) {
    this->write_to_queue_timestamp(timestamp);
    this->write_to_queue_datapoint(data);

    this->switch_buffers();
    // Note that this signifies the total number of entries added to the file, not the number of entries currently in
    // the file
    this->entries_added++;

    // If enough data has been logged for a new index entry, create it
    if ((this->indexfile != NULL) && (this->entries_added % LOG_SIMPLE_INDEX_DENSITY == 1))
      this->write_to_index(timestamp, this->file_break - sizeof(time_t) - sizeof(T));

    this->queue_len = 0;
  }

  /**
   * Read an array of log entries from the chosen time period
   */
  log_slice<circular_log, T> slice(time_t start_ts, ///< [in] Starting point of the chosen time period
    time_t end_ts                                   ///< [in] Endpoint of the chosen time period
  ) {
    return make_log_slice<circular_log, T>(
      this->file,
      this->file_size,
      this->indexfile,
      this->indexfile_size,
      start_ts,
      end_ts,
      -128,
      this->file_break
    );
  }

  /**
   * Read an array of log entries from the chosen time period
   * Write resulting slice into new_file
   */
  log_slice<circular_log, T> slice(time_t start_ts, ///< [in] Starting point of the chosen time period
    time_t end_ts,                                  ///< [in] Endpoint of the chosen time period
    uint8_t *new_file                               ///< [in] File where the log slice is to be written
  ) {
    return make_log_slice<circular_log, T>(
      this->file,
      this->file_size,
      this->indexfile,
      this->indexfile_size,
      start_ts,
      end_ts,
      new_file,
      -128,
      this->file_break
    );
  }

  /**** Static utility functions ****/

  /**
   * Find closest log entry whose timestamp is less than or equal to given timestamp, starting from address file +
   * search_location
   */
  static uint32_t find_log_entry(uint8_t *file, ///< [in] Pointer to the log file from where we search the entry
    uint32_t file_size,                         ///< [in] Size of the log file from where we search the entry
    uint8_t datapoint_size,                     ///< [in] Size of datapoints in the log file in bytes
    time_t timestamp,                           ///< [in] Timestamp to be searched
    uint32_t search_location,                   ///< [in] From which byte in the file does the search start
    bool succeeding                             ///< [in] Do we want the entry containing the timestamp or the entry
                                                ///< after it
  ) {
    uint32_t reading_location = search_location;
    bool done = false;

    while (!done) {
      // If reading location goes out of bounds for the file, circle back to the end of the file

      reading_location = (int64_t) reading_location - (sizeof(time_t) + datapoint_size) >=
        LOG_MANDATORY_DECODE_INFO_SIZE ?
        reading_location - (sizeof(time_t) + datapoint_size) :
        file_size - (sizeof(time_t) + datapoint_size);

      time_t entry_ts;
      std::memcpy(&entry_ts, file + reading_location, sizeof(time_t));

      // The timestamp we are looking for is in this entry
      if (timestamp >= entry_ts) {
        // if we need to find the succeeding entry to the one containing the timestamp instead
        if (succeeding)
          reading_location += sizeof(time_t) + datapoint_size;

        // Exit search loop
        done = true;
      }
    }

    return reading_location;
  }

  /**
   * Find first logged timestamp in the log file
   */
  static time_t find_first_timestamp(uint8_t *file, ///< [in] Pointer to the log file
    uint32_t file_size,                             ///< [in] Size of the log file
    uint32_t file_break                             ///< [in] Break in the log file (separating last logged datapoints
                                                    ///< from first logged ones)
  ) {
    time_t first_ts;
    uint32_t first_ts_location = file_size - file_break < sizeof(time_t) ? LOG_MANDATORY_DECODE_INFO_SIZE : file_break;

    std::memcpy(&first_ts, file + first_ts_location, sizeof(time_t));

    return first_ts;
  }

  /**
   * Find last logged timestamp in the log file
   */
  static time_t find_last_timestamp(uint8_t *file, ///< [in] Pointer to the log file
    uint32_t file_break,                           ///< [in] Break in the log file (separating last logged datapoints
                                                   ///< from first logged ones)
    int8_t resolution                              ///< [in] Resolution of timestamps in the log file
  ) {
    time_t last_ts;

    std::memcpy(&last_ts, file + file_break - sizeof(T) - sizeof(time_t), sizeof(time_t));

    return last_ts;
  }
};
}

#endif
