#include "common.h"
#include "logging.h"

int main() {
    int logsize = 1024;
    // TODO: check for delta size uint8_t
    Log<int, uint8_t> logi = Log<int,uint8_t>();

    int datapoints[] = {12, 13, 14, 15, 2000, 3333, 4690};
    time_t timestamps[] = {1603723663, 1603723700, 1603723720, 1603724000, 1603724000, 1603724100, 1603724900};

    // Expected behaviour: first datapoint is added to first entry
    // next 2 datapoints to the second entry
    // next datapoint to the third entry
    // next 2 datapoints to the fourth entry
    // last datapoint is added to the fifth entry
    for (int i = 0; i < 7; i++) {
        logi.log(datapoints + i, timestamps[i]);
        if (i == 0 || i == 3) {
            logi.flush_buffer();
        }
    }

    Success_Handler();
}
