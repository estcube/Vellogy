#include <cmath>

#include "common.h"
#include "logging.h"

#define PI 3.14159265

static void test1() {
    int logsize = 1024;

    Log<int, uint8_t> logi = Log<int,uint8_t>();

    int datapoints[] = {12, 13, 14, 15, 2000, 3333, 4690};
    time_t timestamps[] = {1603723663, 1603723700, 1603723720, 1603724000, 1603724000, 1603724100, 1603724900};

    // Expected behaviour: first 3 datapoints are added to first entry
    // next 3 datapoints to the second entry
    // last datapoint is added to the third entry
    for (uint8_t i = 0; i < 7; i++) {
        logi.log(datapoints + i, timestamps[i]);
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
}

static void test3() {
    // Metafile containing random decode info
    uint8_t metafile[] = {0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x01};
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
