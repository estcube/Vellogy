#include <vector>
#include <cstdlib>

#include "logging.h"

template <class T>
Log<T>::Log(void* file_location) {
    file = file_location;

    // Initialize the very first entry in the log
    last_entry = (multi_entry_t<T>*)malloc(sizeof(multi_entry_t<T>));
    last_entry->previous = NULL;
    last_entry->offset = 0;
}

template <class T>
Log<T>::Log(void* file_location, int file_size) {
    file = file_location;
    filesize = file_size;

    // Initialize the very first entry in the log
    last_entry = (multi_entry_t<T>*)malloc(sizeof(multi_entry_t<T>));
    last_entry->previous = NULL;
    last_entry->offset = 0;
}

template <class T>
void Log<T>::log(T* data, time_t timestamp) {
    // sizeof? ühe entry täitumine?
    // logi täitumine?
    // erinevat tüüpi entryd?

    // If the last log entry is empty (uninitialized)
    if (last_entry->offset == 0) {
        last_entry->timestamp = timestamp;
        last_entry->data[last_entry->offset] = *data;
        last_entry->deltas[last_entry->offset] = 0;
        last_entry->offset++;
        return;
    }

    // TODO: timestamp resolution full?
    last_entry->data[last_entry->offset] = *data;
    last_entry->deltas[last_entry->offset] = timestamp - last_entry->timestamp;
    last_entry->offset++;

    // If the last entry is full, create a new one
    if (last_entry->offset == BLOCK_SIZE) {
        multi_entry_t<T>* new_entry = (multi_entry_t<T>*)malloc(sizeof(multi_entry_t<T>));
        new_entry->previous = last_entry;
        new_entry->offset = 0;
        last_entry = new_entry;
    }
}
