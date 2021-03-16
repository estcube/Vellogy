#include <cmath>

#include "common.h"
#include "logging.h"

#define PI 3.14159265

static void test1() {
    int logsize = 1024;

    Log<int, uint8_t> logi = Log<int,uint8_t>();

    int datapoints[] = {12, 13, 14, 15, 2000, 3333, 4690};
    time_t timestamps[] = {1603723663, 1603723700, 1603723720, 1603724000, 1603724000, 1603724100, 1603724900};

    // Expected behaviour: first datapoint is added to first entry
    // next 2 datapoints to the second entry
    // next datapoint to the third entry
    // next 2 datapoints to the fourth entry
    // last datapoint is added to the fifth entry
    for (uint8_t i = 0; i < 7; i++) {
        logi.log(datapoints + i, timestamps[i]);
        if (i == 0 || i == 3) {
            logi.flush_buffer();
        }
    }
}

static void test2() {
    double result[360];
    time_t timestamp = 1603723663;

    Log<double, uint8_t> logi = Log<double, uint8_t>();

    for (uint16_t time = 0; time < 360; time++) {
        result[time] = sin((timestamp + time) * PI / 180.0);
        logi.log(result + time, timestamp + time);
    }

    logi.flush_buffer();
}

static void test3() {
    // Metafile containing random decode info and the following internal state info:
    // last timestamp: 1603723663
    // data_added_file: 1 (actually meaning 2)
    // decode_start_position: 0
    // flushed: true
    uint8_t metafile[] = {0x1, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x2, 0x2, 0x2, 0x1, 0x0, 0x0, 0x0, 0x3, 0x3, 0x3, 0x3, 0x1, 0x0, 0x0, 0x0, 0x8f, 0xe1, 0x96, 0x5f, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1};
    uint8_t* file = NULL;

    Log<int, uint8_t> logi = Log<int, uint8_t>(metafile, file);

    int datapoints[] = {14, 15, 2000, 3333, 4690};
    time_t timestamps[] = {1603723720, 1603724000, 1603724000, 1603724100, 1603724900};

    for (uint8_t i = 0; i < 5; i++) {
        logi.log(datapoints + i, timestamps[i]);
    }

    logi.save_meta_info();
}

int main() {
    // test1();
    // test2();
    test3();

    Success_Handler();
}
