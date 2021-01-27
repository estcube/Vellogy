#include "common.h"
#include "logging.h"

int main() {
    int logsize = 1024;
    // TODO: check for delta size uint8_t
    Log<int, uint8_t> logi = Log<int,uint8_t>();

    int datapoints[] = {13, 14, 15, 2000, 3333, 4690};
    time_t timestamps[] = {1603723663, 1603723700, 1603724000, 1603724000, 1603724100, 1603724900};

    // Expected behaviour: first 2 datapoints are added to first entry
    // next 3 datapoints are added to the second entry
    // last datapoint is added to a third entry
    for (int i = 0; i < 6; i++) {
        logi.log(datapoints + i, timestamps[i]);
    }

    Success_Handler();
}
