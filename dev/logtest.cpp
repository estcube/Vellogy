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

static void test4() {
    Log<int, uint8_t> logi = Log<int, uint8_t>();

    int data[100];
    time_t timestamp = 1603723663;

    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        logi.log(data + i, timestamp + i*64);
    }
}

static void test5() {
    // Metafile containing random decode info (and space for additional index entries)
    uint8_t metafile[200] = {
        // decode info
        0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x01, 0x00,
        // number of file entries
        0x18, 0x00, 0x00, 0x00,
        // file size in bytes
        0xb8, 0x02, 0x00, 0x00,
        // number of index entries
        0x05, 0x00, 0x00, 0x00,
        // index entries (each on their own line)
        0x8f, 0xe1, 0x96, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x8f, 0xe6, 0x96, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x91, 0x00, 0x00, 0x00,
        0x8f, 0xeb, 0x96, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x22, 0x01, 0x00, 0x00,
        0x8f, 0xf0, 0x96, 0x5f, 0x00, 0x00, 0x00, 0x00, 0xb3, 0x01, 0x00, 0x00,
        0x8f, 0xf5, 0x96, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x44, 0x02, 0x00, 0x00
    };
    uint8_t* file = NULL;

    Log<int, uint8_t> logi = Log<int, uint8_t>(metafile, file);

    int data[100];
    time_t timestamp = 1603730000;

    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        logi.log(data + i, timestamp + i*64);
    }

    logi.save_meta_info();

    // A log to test whether metainfo was serialized correctly
    Log<int, uint8_t> logi_test = Log<int, uint8_t>(metafile, file);
}

static void test6() {
    uint8_t metafile[200] = {
        // decode info
        0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x01, 0x00,
        // number of file entries
        0x00, 0x00, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // number of index entries
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t file[1024];

    Log<int, uint8_t> logi = Log<int, uint8_t>(metafile, file);

    int data[100];
    time_t timestamp = 1603730000;

    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        logi.log(data + i, timestamp + i*64);
    }

    logi.save_meta_info();

    // A log to test whether metainfo was serialized correctly
    Log<int, uint8_t> logi_test = Log<int, uint8_t>(metafile, file);

    // Exact timestamp tests:
    // Entries in different log and index entries
    log_slice_t slice1 = log_slice<int, uint8_t>(file, metafile + 36, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730064, 1603735056);
    // Entries in different log, but same index entries
    log_slice_t slice2 = log_slice<int, uint8_t>(file, metafile + 36, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730320, 1603730576);
    // Entries in the same log (and index) entry
    log_slice_t slice3 = log_slice<int, uint8_t>(file, metafile + 36, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730064, 1603730128);

    // Random timestamp tests:
    // Entries in different log and index entries
    log_slice_t slice4 = log_slice<int, uint8_t>(file, metafile + 36, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603732103, 1603735077);
    // Entries in different log, but same index entries
    log_slice_t slice5 = log_slice<int, uint8_t>(file, metafile + 36, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730261, 1603730619);
    // Entries in the same log (and index) entry
    log_slice_t slice6 = log_slice<int, uint8_t>(file, metafile + 36, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730901, 1603736130);
    // Should be empty
    log_slice_t slice7 = log_slice<int, uint8_t>(file, metafile + 36, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603734070, 1603734098);
}

int main() {
    // test1();
    // test2();
    // test3();
    // test4();
    // test5();
    test6();

    Success_Handler();
}
