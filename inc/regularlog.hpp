/**
 * @file regularlog.hpp
 *
 * Author: Annika Jaakson
 * Refactored by: Kerdo Kurs
 *
 * This file provides the RegularLog class
 * which is meant to be used in cases where the datapoints logged are not captured periodically
 * but also not far apart in time
 */

#ifndef REGULARLOG_H
#define REGULARLOG_H

#include "baselog.hpp"

namespace eclog {
/**
 * Log class for regular data logging (datapoints are not captured periodically but also not far apart in time)
 *
 * @tparam T datatype of datapoints held in the log
 */
template<class T>
class regular_log : public base_log<T> {
  protected:
  // Logic
  time_t entry_timestamp; ///< Timestamp in the beginning of the last log entry
  uint8_t data_added;     ///< How many datapoints have been added to the queue under the last timestamp
  const int8_t resolution; ///< TS_BASE^resolution is the smallest unit of time that needs to separable

  /**
   * Calculate minimum possible size in bytes for the data queue (maximum size of log entry in bytes)
   */
  constexpr uint32_t min_queue_size() {
    uint32_t queue_size = 0;

    queue_size += sizeof(time_t); // Size of timestamp
    queue_size += sizeof(uint8_t); // Size of data_added
    queue_size += (1 << (sizeof(uint8_t) * CHAR_BIT)) * (sizeof(T) + sizeof(uint8_t)); // 2^{size_of_uint8_t_in_bits} *
                                                                                       // size_of_one_datapoint_with_timedelta

    return queue_size;
  }

  /**
   * Round a given timedelta to a multiple of TS_BASE^resolution and divide by TS_BASE^resolution
   */
  uint16_t scale_timedelta(uint64_t timedelta ///< [in] Timedelta to be scaled
  ) {
    // Relative precision: how much less precise is the timedelta we can log compared to the actual timestamp resolution
    // For example, if TS_RESOLUTION is -4 and this->resolution is -2 and timestamp subseconds are in decimal system,
    // then timedeltas actually saved are 100 times less precise than the timestamp type allows
    uint32_t rel_precision = pow(TS_BASE, abs(this->resolution - TS_RESOLUTION));

    // For example, if timedelta is 7863 and rel_precision is 100, then lower_bound is 7800 and upper_bound is 7900
    uint16_t lower_bound = timedelta / rel_precision * rel_precision;
    uint16_t upper_bound = (timedelta / rel_precision + 1) * rel_precision;

    return (upper_bound - timedelta < timedelta - lower_bound ? upper_bound : lower_bound) / rel_precision;
  }

  /**
   * Write the timestamp of the last datapoint to the data queue
   */
  void write_to_queue_timestamp() {
    this->write_to_queue(&(this->entry_timestamp), sizeof(this->entry_timestamp));
  }

  /**
   * Write a datapoint to the data queue
   */
  void write_to_queue_datapoint(T &datapoint ///< [in] Datapoint to be written to the queue
  ) {
    this->write_to_queue(&datapoint, sizeof(datapoint));
  }

  /**
   * Write a timedelta value to the data queue
   */
  void write_to_queue_timedelta(uint8_t timedelta) {
    this->write_to_queue(&timedelta, sizeof(timedelta));
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
    this->write_to_queue_data_added();
    // A new entry was written to the log file
    this->entries_added++;

    // Create an index entry (write it to indexfile) if enough log entries have been added to the log file
    if ((this->indexfile != NULL) && (this->entries_added % LOG_REGULAR_INDEX_DENSITY == 1))
      this->write_to_index(this->entry_timestamp, this->file_size - this->queue_len);

    // Reset log state
    this->entry_timestamp = 0;
    this->data_added = 0;

    // Reset queue state
    this->queue_len = 0;
  }

  public:
  /**** Constructors ****/

  /**
   * Initialize the log with the given file (no meta- and indexfile)
   */
  regular_log(uint8_t *file, ///< [in] Pointer to the file where datapoints will be saved
    int8_t resolution        ///< [in] Resolution of the timestamps to be saved in the log file
  ) : base_log<T>(file), entry_timestamp{0}, data_added{0}, resolution{resolution} {
    // If this is a brand new file without previous datapoints, write the mandatory decode info into the beginning of
    // the file
    if (!this->file_size) {
      log_file_type_t file_type = LOG_REGULAR;
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
  regular_log(uint8_t *metafile, ///< [in] Pointer to the file where metainfo will be saved
    uint8_t *indexfile,          ///< [in] Pointer to the file where index entries will be saved
    uint8_t *file,               ///< [in] Pointer to the file where datapoints will be saved
    int8_t resolution            ///< [in] Resolution of the timestamps to be saved in the log file
  ) : base_log<T>(metafile, indexfile, file), entry_timestamp{0}, data_added{0}, resolution{resolution} {
    // If this is a brand new file without previous datapoints, write the mandatory decode info into the beginning of
    // the file
    if (!this->file_size) {
      log_file_type_t file_type = LOG_REGULAR;
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
  regular_log(log_slice<regular_log, T> *slice, ///< [in] Log slice containing the data to be copied into the new log
    uint8_t *new_file                           ///< [in] Pointer to the file where slice contents will be copied
  ) : regular_log<T>(new_file, slice->get_resolution()) {
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
  ~regular_log() {
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
    bool new_entry = false;

    if (!this->entry_timestamp ||
      (this->scale_timedelta(timestamp - this->entry_timestamp) >= (1 << (sizeof(uint8_t) * CHAR_BIT))) ||
      (this->data_added >= (1 << (sizeof(uint8_t) * CHAR_BIT)) - 1)) {
      // If one timestamp resolution is full, write to queue how many datapoints were recorded under the previous
      // timestamp
      if (this->entry_timestamp)
        this->end_entry();

      // Create a new timestamp and write it to queue
      this->entry_timestamp = timestamp;
      this->write_to_queue_timestamp();
      // This is a new entry with a new timestamp
      new_entry = true;
    }

    // Write datapoint to queue
    this->write_to_queue_datapoint(data);
    // If time_diff was bigger than 8 bits, then a new entry was created above, so this cast is safe
    uint8_t time_diff = (uint8_t) this->scale_timedelta(timestamp - this->entry_timestamp);
    this->write_to_queue_timedelta(time_diff);

    // The maximum number of datapoints under one timestamp is equal to maximum number of different deltas, 2^(size of
    // uint8_t in bits)
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

  /**
   * Read an array of log entries from the chosen time period
   */
  log_slice<regular_log, T> slice(time_t start_ts, ///< [in] Starting point of the chosen time period
    time_t end_ts                                  ///< [in] Endpoint of the chosen time period
  ) {
    return make_log_slice<regular_log, T>(
      this->file,
      this->file_size,
      this->indexfile,
      this->indexfile_size,
      start_ts,
      end_ts,
      this->resolution
    );
  }

  /**
   * Read an array of log entries from the chosen time period
   * Write resulting slice into new_file
   */
  log_slice<regular_log, T> slice(time_t start_ts, ///< [in] Starting point of the chosen time period
    time_t end_ts,                                 ///< [in] Endpoint of the chosen time period
    uint8_t *new_file                              ///< [in] File where the log slice is to be written
  ) {
    return make_log_slice<regular_log, T>(
      this->file,
      this->file_size,
      this->indexfile,
      this->indexfile_size,
      start_ts,
      end_ts,
      new_file,
      this->resolution
    );
  }

  // Log<T> compress(compression_method_t method); // Compress log with the chosen method
  // Log<T> merge(Log<T> otherLog); // Merge two logs and create a new log

  /**** Utility functions ****/

  /**
   * Write all datapoints in volatile memory to file
   */
  void flush() {
    this->end_entry();
  }

  /**
   * Return resolution of log timestamps
   */
  int8_t get_resolution() {
    return this->resolution;
  }

  /**
   * Signify period change in incoming data on user level
   * Dummy function to make the common Log interface more general
   */
  void period_change() {}

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
      reading_location -= sizeof(uint8_t);
      uint8_t data_added;
      std::memcpy(&data_added, file + reading_location, sizeof(uint8_t));

      // Let's jump ahead and read the timestamp in the beginning of the entry
      uint32_t data_size = (data_added + 1) * (datapoint_size + sizeof(uint8_t));
      reading_location -= data_size + sizeof(time_t);
      time_t entry_ts;
      std::memcpy(&entry_ts, file + reading_location, sizeof(time_t));

      // The timestamp we are looking for is in this entry
      if (timestamp >= entry_ts) {
        // if we need to find the succeeding entry to the one containing the timestamp instead
        if (succeeding)
          reading_location += sizeof(time_t) + data_size + sizeof(uint8_t);

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
    uint8_t last_data_added;

    std::memcpy(&last_data_added, file + file_break - sizeof(uint8_t), sizeof(uint8_t));

    uint32_t last_timedelta;
    std::memcpy(&last_timedelta, file + file_break - 2 * sizeof(uint8_t), sizeof(uint8_t));
    // Relative precision of timedelta (see scale_timedelta function for more info)
    uint32_t rel_precision = pow(TS_BASE, abs(resolution - TS_RESOLUTION));
    last_timedelta *= rel_precision;

    time_t last_ts;
    std::memcpy(
      &last_ts,
      file + file_break - sizeof(uint8_t) - (last_data_added + 1) * (sizeof(T) + sizeof(uint8_t)) -
      sizeof(time_t),
      sizeof(time_t)
    );
    last_ts += last_timedelta;

    return last_ts;
  }
};
}

#endif
