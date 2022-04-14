/**
 * @file periodiclog.hpp
 *
 * Author: Annika Jaakson
 * Refactored by: Kerdo Kurs
 *
 * This file provides the PeriodicLog class
 * which is meant to be used in cases where the datapoints logged are captured periodically
 */

#ifndef PERIODICLOG_H
#define PERIODICLOG_H

#include "baselog.hpp"

namespace eclog {
/**
 * Log class for periodic data logging (datapoints are evenly spaced in time)
 *
 * @tparam T datatype of datapoints held in the log
 */
template<class T>
class periodic_log : public base_log<T> {
  protected:
  time_t entry_timestamp; ///< Timestamp of the first datapoint in the entry
  time_t last_timestamp; ///< Timestamp of the last datapoint in the entry
  uint32_t data_added;   ///< How many datapoints have been added to the queue under the last entry timestamp

  /**
   * Calculate the data queue size that is necessary for holding an entire log entry
   */
  constexpr uint32_t min_queue_size() {
    return sizeof(time_t) * 2 + LOG_PERIODIC_DATAPOINTS_IN_ENTRY * sizeof(T) + sizeof(uint32_t);
  }

  /**
   * Write the timestamp of the last datapoint to the data queue
   */
  void write_to_queue_timestamp() {
    this->write_to_queue(&(this->last_timestamp), sizeof(this->last_timestamp));
  }

  /**
   * Write a datapoint to the data queue
   */
  void write_to_queue_datapoint(T &data ///< [in] Datapoint to be written to the queue
  ) {
    this->write_to_queue(&data, sizeof(data));
  }

  /**
   * Write how many datapoints were added to the last log entry to the data queue
   */
  void write_to_queue_data_added() {
    this->write_to_queue(&(this->data_added), sizeof(this->data_added));
    this->switch_buffers();
  }

  /**
   * End a log entry and write it to file, reset log state
   */
  void end_entry() {
    this->write_to_queue_timestamp();
    this->write_to_queue_data_added();

    // A new entry was written to the log file
    this->entries_added++;

    // Create an index entry (write it to indexfile) if enough log entries have been added to the log file
    if ((this->indexfile != NULL) && (this->entries_added % LOG_PERIODIC_INDEX_DENSITY == 1))
      this->write_to_index(this->entry_timestamp, this->file_size - this->queue_len);

    // Reset log state
    this->entry_timestamp = 0;
    this->last_timestamp = 0;
    this->data_added = 0;

    // Reset queue state
    this->queue_len = 0;
  }

  public:
  /**** Constructors ****/

  /**
   * Initialize the log with the given file (no meta- and indexfile)
   */
  periodic_log(uint8_t *file ///< [in] Pointer to the file where datapoints will be saved
  ) : base_log<T>(file), entry_timestamp{0}, last_timestamp{0}, data_added{0} {
    // If this is a brand new file without previous datapoints, write the mandatory decode info into the beginning of
    // the file
    if (!this->file_size) {
      log_file_type_t file_type = LOG_PERIODIC;
      log_data_type_t data_type = LOG_INT32_T; // TODO: non-dummy value

      std::memcpy(this->file + this->file_size, &file_type, sizeof(log_file_type_t));
      this->file_size += sizeof(log_file_type_t);
      // TODO: non-dummy value
      std::memcpy(this->file + this->file_size, &data_type, sizeof(log_data_type_t));
      this->file_size += sizeof(log_data_type_t);
    }

    this->data_queue = (uint8_t *) pvPortMalloc(this->min_queue_size());
    this->double_buffer = (uint8_t *) pvPortMalloc(this->min_queue_size());
  }

  /**
   * Initialize the log (from a metafile) held in the file given
   */
  periodic_log(uint8_t *metafile, ///< [in] Pointer to the file where metainfo will be saved
    uint8_t *indexfile,           ///< [in] Pointer to the file where index entries will be saved
    uint8_t *file                 ///< [in] Pointer to the file where datapoints will be saved
  ) : base_log<T>(metafile, indexfile, file), entry_timestamp{0}, last_timestamp{0}, data_added{0} {
    // If this is a brand new file without previous datapoints, write the mandatory decode info into the beginning of
    // the file
    if (!this->file_size) {
      log_file_type_t file_type = LOG_PERIODIC;
      log_data_type_t data_type = LOG_INT32_T; // TODO: non-dummy value

      std::memcpy(this->file + this->file_size, &file_type, sizeof(log_file_type_t));
      this->file_size += sizeof(log_file_type_t);
      // TODO: non-dummy value
      std::memcpy(this->file + this->file_size, &data_type, sizeof(log_data_type_t));
      this->file_size += sizeof(log_data_type_t);
    }

    this->data_queue = (uint8_t *) pvPortMalloc(this->min_queue_size());
    this->double_buffer = (uint8_t *) pvPortMalloc(this->min_queue_size());
  }

  /**
   * Initialize the log from a log slice
   */
  periodic_log(log_slice<periodic_log, T> *slice, ///< [in] Log slice containing the data to be copied into the new log
    uint8_t *new_file ///< [in] Pointer to the file where slice contents will be copied
  ) : periodic_log<T>(new_file) {
    // Copy slice into the file provided
    std::memcpy(
      this->file + this->file_size,
      slice->get_file() + slice->get_start_location(),
      slice->get_end_location() - slice->get_start_location()
    );
    this->file_size += slice->get_end_location() - slice->get_start_location();
  }

  /**
   * Free allocated buffers on object destruction
   */
  ~periodic_log() {
    vPortFree(this->data_queue);
    vPortFree(this->double_buffer);
  }

  /**** Main functionality ****/

  /**
   * Log data, attach timestamp in function
   */
  void log(
    T &data ///< [in] Datapoint to be logged
  );

  /**
   * Log data with a given timestamp in the file
   */
  void log(T &data,  ///< [in] Datapoint to be logged
    time_t timestamp ///< [in] Capturing time of the datapoint
  ) {
    this->last_timestamp = timestamp;

    // Start logging under a new timestamp
    if (!this->entry_timestamp) {
      this->entry_timestamp = timestamp;
      this->write_to_queue_timestamp();
    }

    this->write_to_queue_datapoint(data);
    this->data_added++;

    // If entry is full, write it to memory
    if (this->data_added >= LOG_PERIODIC_DATAPOINTS_IN_ENTRY)
      this->end_entry();
  }

  /**
   * Read an array of log entries from the chosen time period
   */
  log_slice<periodic_log, T> slice(time_t start_ts, ///< [in] Starting point of the chosen time period
    time_t end_ts                                   ///< [in] Endpoint of the chosen time period
  ) {
    return make_log_slice<periodic_log, T>(
      this->file,
      this->file_size,
      this->indexfile,
      this->indexfile_size,
      start_ts,
      end_ts
    );
  }

  /**
   * Read an array of log entries from the chosen time period
   * Write resulting slice into new_file
   */
  log_slice<periodic_log, T> slice(time_t start_ts, ///< [in] Starting point of the chosen time period
    time_t end_ts,                                  ///< [in] Endpoint of the chosen time period
    uint8_t *new_file                               ///< [in] File where the log slice is to be written
  ) {
    return make_log_slice<periodic_log, T>(
      this->file,
      this->file_size,
      this->indexfile,
      this->indexfile_size,
      start_ts,
      end_ts,
      new_file
    );
  }

  /**** Utility functions ****/

  /**
   * Write all datapoints in volatile memory to file
   */
  void flush() {
    this->end_entry();
  }

  /**
   * Return resolution of log timestamps
   * Dummy function to make the common Log interface more general
   */
  int8_t get_resolution() {
    return -128;
  }

  /**
   * Signify period change in incoming data on user level
   */
  void period_change() {
    this->end_entry();
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
      reading_location -= sizeof(uint32_t);
      uint32_t data_added;
      std::memcpy(&data_added, file + reading_location, sizeof(uint32_t));

      // Let's jump ahead and read the timestamp in the beginning of the entry
      uint32_t data_size = data_added * datapoint_size;
      reading_location -= data_size + 2 * sizeof(time_t);
      time_t entry_ts;
      std::memcpy(&entry_ts, file + reading_location, sizeof(time_t));

      // The timestamp we are looking for is in this entry
      if (timestamp >= entry_ts) {
        // if we need to find the succeeding entry to the one containing the timestamp instead
        if (succeeding)
          reading_location += 2 * sizeof(time_t) + data_size + sizeof(uint32_t);

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

    std::memcpy(&first_ts, file + LOG_MANDATORY_DECODE_INFO_SIZE, sizeof(time_t));

    return first_ts;
  }

  /**
   * Find last logged timestamp in the log file
   */
  static time_t find_last_timestamp(uint8_t *file, ///< [in] Pointer to the log file
    uint32_t file_break,                           ///< [in] Size of the log file
    int8_t resolution                              ///< [in] Resolution of timestamps in the log file
  ) {
    time_t last_ts;

    std::memcpy(&last_ts, file + file_break - sizeof(uint32_t) - sizeof(time_t), sizeof(time_t));

    return last_ts;
  }
};
}

#endif
