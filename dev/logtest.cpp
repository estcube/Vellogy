#include <cmath>

#include "log.hpp"

#include "common.h"

#define PI 3.14159265

static void test1() {
    uint8_t file[1024];
    eclog::regular_log<int> logi = eclog::regular_log<int>(file, -3);
    eclog::log log1 = eclog::log(&logi);

    int datapoints[] = {12, 13, 14, 15, 2000, 3333, 4690};
    time_t timestamps[] = {1603723663, 1603723700, 1603723720, 1603724000, 1603724000, 1603724100, 1603724900};

    // Expected behaviour: first 3 datapoints are added to first entry
    // next 3 datapoints to the second entry
    // last datapoint is added to the third entry
    for (uint8_t i = 0; i < 7; i++) {
        log1.write(datapoints[i], timestamps[i]);
    }
}

static void test2() {
    double result[360];
    time_t timestamp = 1603723663;

    uint8_t file[1024];
    eclog::regular_log<double> logi = eclog::regular_log<double>(file, -3);
    eclog::log log2 = eclog::log(&logi);

    // Save sine data (to be plotted later)
    for (uint16_t time = 0; time < 360; time++) {
        result[time] = sin((timestamp + time) * PI / 180.0);
        log2.write(result[time], timestamp + time);
    }
}

static void test3() {
    // Metafile containing random decode info
    uint8_t metafile[] = {
        // Decode info (random)
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // File size
        0x00, 0x00, 0x00, 0x00,
        // Indexfile size
        0x00, 0x00, 0x00, 0x00
    };

    uint8_t indexfile[128];
    uint8_t file[1024];

    eclog::regular_log<int> logi = eclog::regular_log<int>(metafile, indexfile, file, -3);
    eclog::log log3 = eclog::log(&logi);

    int datapoints[] = {14, 15, 2000, 3333, 4690};
    time_t timestamps[] = {1603723720, 1603724000, 1603724000, 1603724100, 1603724900};

    for (uint8_t i = 0; i < 5; i++) {
        log3.write(datapoints[i], timestamps[i]);
    }

    log3.save_meta_info();
}

static void test4() {
    uint8_t file[1024];
    eclog::regular_log<int> logi = eclog::regular_log<int>(file, -3);
    eclog::log log4 = eclog::log(&logi);

    int data[100];
    time_t timestamp = 1603723663;

    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        log4.write(data[i], timestamp + i*64);
    }
}

static void test5() {
    // Test restoring log from existing metafile, indexfile and file

    // Metafile containing random decode info
    uint8_t metafile[200] = {
        // decode info
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // file size in bytes
        0xba, 0x02, 0x00, 0x00,
        // indexfile size in bytes
        0x3c, 0x00, 0x00, 0x00,
    };
    uint8_t indexfile[256] = {
        // index entries (each on their own line)
        0x8f, 0xe1, 0x96, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x8f, 0xe6, 0x96, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x91, 0x00, 0x00, 0x00,
        0x8f, 0xeb, 0x96, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x22, 0x01, 0x00, 0x00,
        0x8f, 0xf0, 0x96, 0x5f, 0x00, 0x00, 0x00, 0x00, 0xb3, 0x01, 0x00, 0x00,
        0x8f, 0xf5, 0x96, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x44, 0x02, 0x00, 0x00
    };
    uint8_t file[2048];

    // We'll see if the metafile was read correctly
    eclog::regular_log<int> logi = eclog::regular_log<int>(metafile, indexfile, file, -3);
    eclog::log log5 = eclog::log(&logi);

    int data[100];
    time_t timestamp = 1603730000;

    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        log5.write(data[i], timestamp + i*64);
    }

    log5.save_meta_info();

    // A log to test whether metainfo was serialized correctly
    eclog::regular_log<int> logi_test = eclog::regular_log<int>(metafile, indexfile, file, -3);
}

static void test6() {
    // Test log slicing

    uint8_t metafile[200] = {
        // decode info (random)
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // index file size in bytes
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t indexfile[128];
    uint8_t file[1024];

    eclog::regular_log<int> logi = eclog::regular_log<int>(metafile, indexfile, file, -3);
    eclog::log log6 = eclog::log(&logi);

    int data[101];
    time_t timestamp = 1603730000;

    // Create a log file with 25 entries, each entry with 4 datapoints (size of one full log entry is 29 bytes)
    // One extra datapoint shall remain in volatile memory
    for (uint8_t i = 0; i < 101; i++) {
        data[i] = i*100;
        log6.write(data[i], timestamp + i*64);
    }

    log6.save_meta_info();

    // A log to test whether metainfo was serialized correctly
    eclog::regular_log<int> logi_test = eclog::regular_log<int>(metafile, indexfile, file, -3);

    // Exact timestamp tests:
    // Entries in different log and index entries
    eclog::log_slice<eclog::regular_log, int> slice1 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730064, 1603735056, -3);  // Expected {2, 582}
    // Entries in different log, but same index entries
    eclog::log_slice<eclog::regular_log, int> slice2 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730320, 1603730576, -3);  // Expected {31, 89}
    // Entries in the same log (and index) entry
    eclog::log_slice<eclog::regular_log, int> slice3 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730064, 1603730128, -3);  // Expected {2, 31}

    // Random timestamp tests:
    // Entries in different log and index entries
    eclog::log_slice<eclog::regular_log, int> slice4 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603732103, 1603735077, -3);  // Expected {234, 582}
    eclog::log_slice<eclog::regular_log, int> alt4 = log6.slice(1603732103, 1603735077);
    // Entries in different log, but same index entries
    eclog::log_slice<eclog::regular_log, int> slice5 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730261, 1603730619, -3);  // Expected {31, 89}
    eclog::log_slice<eclog::regular_log, int> alt5 = log6.slice(1603730261, 1603730619);
    // A big slice
    eclog::log_slice<eclog::regular_log, int> slice6 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730901, 1603736130, -3);  // Expected {89, 698}
    eclog::log_slice<eclog::regular_log, int> alt6 = log6.slice(1603730901, 1603736130);
    // Entries in consecutive log entries
    eclog::log_slice<eclog::regular_log, int> slice7 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603734070, 1603734098, -3);  // Expected {437, 495}
    eclog::log_slice<eclog::regular_log, int> alt7 = log6.slice(1603734070, 1603734098);

    // Edge cases
    // Both timestamps in the very last entry of the log file
    eclog::log_slice<eclog::regular_log, int> slice8 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603736170, 1603736330, -3);  // Expected {698, 727}
    // Timestamps are entry timestamps
    eclog::log_slice<eclog::regular_log, int> slice9 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730512, 1603736144, -3);  // Expected {60, 727}
    // Timestamps are entry timestamps
    eclog::log_slice<eclog::regular_log, int> slice10 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603736144, 1603736336, -3);  // Expected {698, 727}
    // Timestamps are index entry timestamps
    eclog::log_slice<eclog::regular_log, int> slice11 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603732560, 1603735120, -3);  // Expected {292, 611}
    // End timestamp conincides with middle timestamp of the index
    eclog::log_slice<eclog::regular_log, int> slice12 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730512, 1603732560, -3);  // Expected {60, 321}
    // Index only contains one (full) entry
    eclog::log_slice<eclog::regular_log, int> slice13 = eclog::make_log_slice<eclog::regular_log, int>(file, 29 * 5 + 2, indexfile, 1 * (sizeof(time_t) + sizeof(uint32_t)), 1603730011, 1603730200, -3);  // Expected {2, 31}
    // Index only contains one (half) entry
    eclog::log_slice<eclog::regular_log, int> slice14 = eclog::make_log_slice<eclog::regular_log, int>(file, 29 * 3 + 2, indexfile, 1 * (sizeof(time_t) + sizeof(uint32_t)), 1603730003, 1603730030, -3);  // Expected {2, 31}

    // (Semi-)bad/pointless things the user can do
    // Boundary timestamps out of bounds
    eclog::log_slice<eclog::regular_log, int> slice15 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 10, 2002002002, -3);  // Expected {2, 727}
    // End timestamp smaller than start timestamp
    eclog::log_slice<eclog::regular_log, int> slice16 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 2002002002, 10, -3);  // Expected {2, 727}
    // End timestamp equal to start timestamp
    eclog::log_slice<eclog::regular_log, int> slice17 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730512, 1603730512, -3);  // Expected {60, 89}

    uint8_t new_file[512];
    eclog::log<eclog::regular_log, int> new_log = slice4.create_log(new_file);

    uint8_t slice18_file[64];
    eclog::log_slice<eclog::regular_log, int> slice18_withfile = log6.slice(1603730512, 1603730512, slice18_file); // Expected {60, 89}

    vPortFree(new_log.get_obj());
}

static void test7() {
    // Test repeating timestamps

    uint8_t metafile[200] = {
        // decode info (random)
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // index file size in bytes
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t indexfile[128];
    uint8_t file[1536];
    eclog::regular_log<int> logi = eclog::regular_log<int>(metafile, indexfile, file, -3);
    eclog::log log7 = eclog::log(&logi);

    int data[300];
    time_t timestamp = 1603723663;

    // Test an edge case where all the datapoints have the same timestamp
    for (uint16_t i = 0; i < 300; i++) {
        data[i] = i*100;
        log7.write(data[i], timestamp);
    }
}

static void test8() {
    // Test optionality of indexfile

    uint8_t file[1024];
    eclog::regular_log<int> logi = eclog::regular_log<int>(file, -3);  // Special constructor
    eclog::log log8 = eclog::log(&logi);

    int data[100];
    time_t timestamp = 1603730000;

    // No index entries should be created after this loop
    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        log8.write(data[i], timestamp + i*64);
    }

    // Should have no effect
    log8.save_meta_info();

    // Should work as expected
    eclog::log_slice<eclog::regular_log, int> slice1 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), NULL, 0, 1603732103, 1603735077, -3);  // Expected {234, 582}
    eclog::log_slice<eclog::regular_log, int> slice2 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), NULL, 0, 1603730261, 1603730619, -3);  // Expected {31, 89}
    eclog::log_slice<eclog::regular_log, int> slice3 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), NULL, 0, 1603730901, 1603736130, -3);  // Expected {89, 698}
    eclog::log_slice<eclog::regular_log, int> slice4 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), NULL, 0, 1603734070, 1603734098, -3);  // Expected {437, 495}
}

static void test9() {
    // Test flushing log when entry is not yet full

    uint8_t file[1024];
    eclog::regular_log<int> logi = eclog::regular_log<int>(file, -3);
    eclog::log log9 = eclog::log(&logi);

    int data[100];
    time_t timestamp = 1603723700;

    for (uint16_t i = 0; i < 50; i++) {
        data[i] = i*100;
        log9.write(data[i], timestamp + i * 64);
    }

    // After this, a new entry with only 2 datapoints should be written to log file
    log9.flush();

    // After this loop, 12 new entries should be created and volatile memory should hold 2 datapoints
    for (uint16_t i = 50; i < 100; i++) {
        data[i] = i*100;
        log9.write(data[i], timestamp + i * 64);
    }
}

static void test10() {
    // Test different resolutions for log

    uint8_t file1[1024];
    eclog::regular_log<int> logi1 = eclog::regular_log<int>(file1, -3);
    eclog::log log10a = eclog::log(&logi1);

    int data[100];
    time_t timestamp = 1603723700;

    // Expected behaviour: all timedeltas are logged in their original form
    for (uint16_t i = 0; i < 50; i++) {
        data[i] = i*100;
        log10a.write(data[i], timestamp + i * 4);
    }

    uint8_t file2[1024];
    eclog::regular_log<int> logi2 = eclog::regular_log<int>(file2, -2);
    eclog::log log10b = eclog::log(&logi2);

    // Expected behaviour: timedeltas are 0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6, 6, ...
    // (these are the rounded tens of the actual timedeltas)
    for (uint16_t i = 0; i < 50; i++) {
        data[i] = i*100;
        log10b.write(data[i], timestamp + i * 4);
    }
    // Since timedeltas are compressed, the entry won't be full after this loop, so we need to end it manually to see changes in file
    log10b.flush();

    // Test slicing with resolution reduced 10 times
    eclog::log_slice<eclog::regular_log, int> sliceb1 = eclog::make_log_slice<eclog::regular_log, int>(file2, log10b.get_file_size(), NULL, 0, 1603720000, 1603930000, -2);  // Expected {2, 261}
    eclog::log_slice<eclog::regular_log, int> sliceb2 = eclog::make_log_slice<eclog::regular_log, int>(file2, log10b.get_file_size(), NULL, 0, 1603723789, 1603723801, -2);  // Expected {2, 261}

    uint8_t file3[1024];
    eclog::regular_log<int> logi3 = eclog::regular_log<int>(file3, -1);
    eclog::log log10c = eclog::log(&logi3);

    // Expected behaviour: timedeltas are 0 (4 times), 1 (6 times), 2 (6 times) and so on
    // (these are the rounded hundreds of the actual timedeltas)
    for (uint16_t i = 0; i < 50; i++) {
        data[i] = i*100;
        log10c.write(data[i], timestamp + i * 16);
    }
    // Since timedeltas are compressed, the entry won't be full after this loop, so we need to end it manually to see changes in file
    log10c.flush();

    // Test slicing with resolution reduced 100 times
    eclog::log_slice<eclog::regular_log, int> slicec1 = eclog::make_log_slice<eclog::regular_log, int>(file3, log10b.get_file_size(), NULL, 0, 1603720000, 1603930000, -1);  // Expected {2, 261}
    eclog::log_slice<eclog::regular_log, int> slicec2 = eclog::make_log_slice<eclog::regular_log, int>(file2, log10b.get_file_size(), NULL, 0, 1603723789, 1603723801, -1);  // Expected {2, 261}

    uint8_t file4[1024];
    eclog::regular_log<int> logi4 = eclog::regular_log<int>(file4, 0);
    eclog::log log10d = eclog::log(&logi4);

    // Expected behaviour: timedeltas are 0 (2 times), 1 (4 times), 2 (4 times), 3 (4 times) and so on
    // (these are the rounded thousands of the actual timedeltas)
    for (uint16_t i = 0; i < 50; i++) {
        data[i] = i*100;
        log10d.write(data[i], timestamp + i * 256);
    }
    // Since timedeltas are compressed, the entry won't be full after this loop, so we need to end it manually to see changes in file
    log10d.flush();

    // Test slicing with resolution reduced 1000 times
    const int8_t res = 0;
    eclog::log_slice<eclog::regular_log, int> sliced1 = eclog::make_log_slice<eclog::regular_log, int>(file4, log10b.get_file_size(), NULL, 0, 1603720000, 1603930000, res);  // Expected {2, 261}
    eclog::log_slice<eclog::regular_log, int> sliced2 = eclog::make_log_slice<eclog::regular_log, int>(file2, log10b.get_file_size(), NULL, 0, 1603723789, 1603723801, res);  // Expected {2, 261}
}

static void test11() {
    // Test more slicing with reduced resolution and indexfile

    uint8_t metafile[200] = {
        // decode info (random)
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // index file size in bytes
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t indexfile[256];
    uint8_t file[1024];

    eclog::regular_log<int> logi = eclog::regular_log<int>(metafile, indexfile, file, -1);
    eclog::log log11 = eclog::log(&logi);

    int data[100];
    time_t timestamp = 1603730000;

    // After this loop, 24 log entries should be written to file and one should be in volatile memory
    // Saved timedeltas should be 0, 64, 128 and so on
    for (uint16_t i = 0; i < 100; i++) {
        data[i] = i*100;
        log11.write(data[i], timestamp + i * 6400);
    }

    // Slice that contains the whole file
    eclog::log_slice<eclog::regular_log, int> slice1 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), NULL, 0, 1603721111, 1610001616, -1);  // Expected {2, 698}
    eclog::log_slice<eclog::regular_log, int> slice1_indexed = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603721111, 1610001616, -1);
    // eclog::entries in different log, but same index entries
    eclog::log_slice<eclog::regular_log, int> slice2 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), NULL, 0, 1603765511, 1603839900, -1);  // Expected {31, 147}
    eclog::log_slice<eclog::regular_log, int> slice2_indexed = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603765511, 1603839900, -1);
    // Entries in the same log (and index) entry
    eclog::log_slice<eclog::regular_log, int> slice3 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), NULL, 0, 1604134487, 1604139555, -1);  // Expected {437, 466}
    eclog::log_slice<eclog::regular_log, int> slice3_indexed = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1604134487, 1604139555, -1);
    // Entries in different index entries
    eclog::log_slice<eclog::regular_log, int> slice4 = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), NULL, 0, 1603736499, 1604254803, -1);  // Expected {2, 611}
    eclog::log_slice<eclog::regular_log, int> slice4_indexed = eclog::make_log_slice<eclog::regular_log, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603736499, 1604254803, -1);
}

static void test12() {
    // Test SimpleLog functionality

    // First with indexfile
    // Metafile containing random decode info
    uint8_t metafile[200] = {
        // decode info
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // indexfile size in bytes
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t indexfile[256];
    uint8_t file[2048];

    eclog::simple_log<int> logi = eclog::simple_log<int>(metafile, indexfile, file);
    eclog::log log12 = eclog::log(&logi);

    int data[100];
    time_t timestamp = 1603740000;

    // Expected behaviour: the log file is 1200 bytes larger after this loop and 20 index entries are created
    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        log12.write(data[i], timestamp + i*64);
    }

    // Test whole file slice
    eclog::log_slice<eclog::simple_log, int> slice1 = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), NULL, 0, 1603740000, 1603746400);  // Expected {2, 1202}
    eclog::log_slice<eclog::simple_log, int> slice1_indexed = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), indexfile, 240, 1603740000, 1603746400);
    eclog::log_slice<eclog::simple_log, int> alt1 = log12.slice(1603740000, 1603746400);
    // Test timestamps out of bounds
    eclog::log_slice<eclog::simple_log, int> slice2 = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), NULL, 0, 1603730000, 1603766400);  // Expected {2, 1202}
    eclog::log_slice<eclog::simple_log, int> slice2_indexed = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), indexfile, 240, 1603730000, 1603766400);
    eclog::log_slice<eclog::simple_log, int> alt2 = log12.slice(1603730000, 1603766400);
    // Test random timestamps in different index entries
    eclog::log_slice<eclog::simple_log, int> slice3 = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), NULL, 0, 1603740131, 1603744681);  // Expected {26, 890}
    eclog::log_slice<eclog::simple_log, int> slice3_indexed = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), indexfile, 240, 1603740131, 1603744681);
    eclog::log_slice<eclog::simple_log, int> alt3 = log12.slice(1603740131, 1603744681);
    // Test exact entry timestamps
    eclog::log_slice<eclog::simple_log, int> slice4 = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), NULL, 0, 1603740640, 1603742560);  // Expected {122, 494}
    eclog::log_slice<eclog::simple_log, int> slice4_indexed = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), indexfile, 240, 1603740640, 1603742560);
    eclog::log_slice<eclog::simple_log, int> alt4 = log12.slice(1603740640, 1603742560);
    // Test random timestamps in same index entry
    eclog::log_slice<eclog::simple_log, int> slice5 = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), NULL, 0, 1603740972, 1603741099);  // Expected {182, 218}
    eclog::log_slice<eclog::simple_log, int> slice5_indexed = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), indexfile, 240, 1603740972, 1603741099);
    eclog::log_slice<eclog::simple_log, int> alt5 = log12.slice(1603740972, 1603741099);
    // Test random timestamps in same log entry
    eclog::log_slice<eclog::simple_log, int> slice6 = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), NULL, 0, 1603740972, 1603740980);  // Expected {182, 194}
    eclog::log_slice<eclog::simple_log, int> slice6_indexed = eclog::make_log_slice<eclog::simple_log, int>(file, logi.get_file_size(), indexfile, 240, 1603740972, 1603740980);
    eclog::log_slice<eclog::simple_log, int> alt6 = log12.slice(1603740972, 1603740980);

    uint8_t new_file[512];
    eclog::log<eclog::simple_log, int> new_log = alt5.create_log(new_file);

    uint8_t slice7_file[512];
    eclog::log_slice<eclog::simple_log, int> slice7_withfile = log12.slice(1603740640, 1603742560, slice7_file); // Expected {122, 494}

    vPortFree(new_log.get_obj());
}

static void test13() {
    // Test PeriodicLog functionality
    // NOTE: LOG_PERIODIC_DATAPOINTS_IN_ENTRY is set to 20

    // First without indexfile
    uint8_t file1[1024];

    eclog::periodic_log<int> logi1 = eclog::periodic_log<int>(file1);
    eclog::log log13a = eclog::log(&logi1);

    int data1[200];
    time_t timestamp1 = 1626771000;

    // Expected behaviour: 4 log entries are created
    for (uint8_t i = 0; i < 97; i++) {
        data1[i] = i;
        log13a.write(data1[i], timestamp1);
        timestamp1 += 100;
    }

    // Expected behaviour: 5th log entry is created
    log13a.period_change();

    // Expected behaviour: 3 more log entries are created and 6 datapoints remain in the queue
    for (uint8_t i = 97; i < 163; i++) {
        data1[i] = i;
        log13a.write(data1[i], timestamp1);
        timestamp1 += 50;
    }

    // Expected behaviour: 9th log entry is created
    log13a.flush();

    // Now with indexfile
    // Metafile containing random decode info
    uint8_t metafile2[] = {
        // decode info
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // indexfile size in bytes
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t indexfile2[256];
    uint8_t file2[2048];

    eclog::periodic_log<int> logi2 = eclog::periodic_log<int>(metafile2, indexfile2, file2);
    eclog::log log13b = eclog::log(&logi2);

    int data2[300];
    time_t timestamp2 = 1626771000;

    // Expected behaviour: 5 log entries and 1 index entry are created
    for (uint8_t i = 0; i < 111; i++) {
        data2[i] = i;
        log13b.write(data2[i], timestamp2);
        timestamp2 += 50;
    }

    // Expected behaviour: 6th log entry and 2nd index entry are created
    log13b.period_change();

    // Expected behaviour: 6 more log entries (12 in total now) and 3rd index entry are created
    for (uint8_t i = 111; i < 238; i++) {
        data2[i] = i;
        log13b.write(data2[i], timestamp2);
        timestamp2 += 100;
    }

    // Expected behaviour: 13th log entry is created
    log13b.flush();

    // Test logging into a file that already has entries
    log13b.save_meta_info();
    eclog::periodic_log<int> logi3 = eclog::periodic_log<int>(metafile2, indexfile2, file2);
    eclog::log log13c = eclog::log(&logi3);

    int data3[100];
    time_t timestamp3 = 1626771000;

    // Expected behaviour: 3 more log entries and 4th index entry are created
    for (uint8_t i = 0; i < 63; i++) {
        data3[i] = i;
        log13c.write(data3[i], timestamp3);
        timestamp3 += 32;
    }

    // Expected behaviour: 17th log entry is created
    log13c.flush();
}

static void test14() {
    // Test PeriodicLog slicing functionality
    // NOTE: LOG_PERIODIC_DATAPOINTS_IN_ENTRY is set to 20

    uint8_t metafile[] = {
        // decode info
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // indexfile size in bytes
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t indexfile[256];
    uint8_t file[1536];

    eclog::periodic_log<int> logi = eclog::periodic_log<int>(metafile, indexfile, file);
    eclog::log log14 = eclog::log(&logi);

    int data[300];
    time_t timestamp = 1626771000;

    // Expected behaviour: 5 log entries and 1 index entry are created
    for (int i = 0; i < 114; i++) {
        data[i] = i;
        log14.write(data[i], timestamp);
        timestamp += 25;
    }

    // Expected behaviour: another log and index entry are created
    log14.period_change();

    // Expected behaviour: 5 more log entries and 1 more index entry are created
    for (int i = 114; i < 224; i++) {
        data[i] = i;
        log14.write(data[i], timestamp);
        timestamp += 50;
    }

    // Expected behaviour: another log entry is created
    log14.flush();

    // Expected behaviour: 2 more log entries are created
    for (int i = 224; i < 264; i++) {
        data[i] = i;
        log14.write(data[i], timestamp);
        timestamp += 50;
    }

    // By now, there should be a total of 14 log entries and 3 index entries

    // Slicing tests
    // Test whole file slice
    eclog::log_slice<eclog::periodic_log, int> slice1 = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), NULL, 0, 1626771009, 1626781280);  // Expected {2, 1338}
    eclog::log_slice<eclog::periodic_log, int> slice1_indexed = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), indexfile, 36, 1626771009, 1626781280);
    eclog::log_slice<eclog::periodic_log, int> alt1 = log14.slice(1626771009, 1626781280);
    // Test timestamps out of bounds
    eclog::log_slice<eclog::periodic_log, int> slice2 = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), NULL, 0, 1626770000, 1636779999);  // Expected {2, 1338}
    eclog::log_slice<eclog::periodic_log, int> slice2_indexed = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), indexfile, 36, 1626770000, 1636779999);
    eclog::log_slice<eclog::periodic_log, int> alt2 = log14.slice(1626770000, 1636779999);
    // Test random timestamps in different index entries
    eclog::log_slice<eclog::periodic_log, int> slice3 = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), NULL, 0, 1626771549, 1626774355);  // Expected {102, 678}
    eclog::log_slice<eclog::periodic_log, int> slice3_indexed = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), indexfile, 36, 1626771549, 1626774355);
    eclog::log_slice<eclog::periodic_log, int> alt3 = log14.slice(1626771549, 1626774355);
    // Test exact entry timestamps
    eclog::log_slice<eclog::periodic_log, int> slice4 = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), NULL, 0, 1626773500, 1626779300);  // Expected {502, 1138}
    eclog::log_slice<eclog::periodic_log, int> slice4_indexed = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), indexfile, 36, 1626773500, 1626779300);
    eclog::log_slice<eclog::periodic_log, int> alt4 = log14.slice(1626773500, 1626779300);
    // Test random timestamps in same index entry
    eclog::log_slice<eclog::periodic_log, int> slice5 = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), NULL, 0, 1626775962, 1626777003);  // Expected {778, 978}
    eclog::log_slice<eclog::periodic_log, int> slice5_indexed = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), indexfile, 36, 1626775962, 1626777003);
    eclog::log_slice<eclog::periodic_log, int> alt5 = log14.slice(1626775962, 1626777003);
    // Test random timestamps in same log entry
    eclog::log_slice<eclog::periodic_log, int> slice6 = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), NULL, 0, 1626779511, 1626779857);  // Expected {1138, 1238}
    eclog::log_slice<eclog::periodic_log, int> slice6_indexed = eclog::make_log_slice<eclog::periodic_log, int>(file, log14.get_file_size(), indexfile, 36, 1626779511, 1626779857);
    eclog::log_slice<eclog::periodic_log, int> alt6 = log14.slice(1626779511, 1626779857);

    uint8_t slice7_file[102];
    eclog::log_slice<eclog::periodic_log, int> slice7_withfile = log14.slice(1626779511, 1626779857, slice7_file); // Expected {1138, 1238}
}

static void test15() {
    // Test (simple) CircularLog functionality

    // First without indexfile
    uint8_t file1[1024];

    eclog::circular_log<int> logi1 = eclog::circular_log<int>(file1, 5 * (sizeof(time_t) + sizeof(int)) + LOG_MANDATORY_DECODE_INFO_SIZE);
    eclog::log log15 = eclog::log(&logi1);

    int data1[200];
    time_t timestamp1 = 1626771000;

    // Expected behaviour: log file holds the last 5 logged datapoints at all times
    for (uint8_t i = 0; i < 50; i++) {
        data1[i] = i;
        log15.write(data1[i], timestamp1);
        timestamp1 += 100;
    }
}

static void test16() {
    // Test (simple) CircularLog slicing functionality

    uint8_t metafile[] = {
        // decode info
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // indexfile size in bytes
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t indexfile[256];
    uint8_t file[1024];

    eclog::circular_log<int> logi = eclog::circular_log<int>(metafile, indexfile, file, 50 * (sizeof(time_t) + sizeof(int)) + LOG_MANDATORY_DECODE_INFO_SIZE);
    eclog::log log16 = eclog::log(&logi);

    int data[200];
    time_t timestamp = 1626771000;

    // Expected behaviour: log file holds the last 50 logged datapoints at all times
    for (uint8_t i = 0; i < 78; i++) {
        data[i] = i;
        log16.write(data[i], timestamp);
        timestamp += 16;
    }

    // Slicing tests
    // Test whole file slice
    eclog::log_slice<eclog::circular_log, int> slice1 = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), NULL, 0, 1626771450, 1626772238, -128, 338);  // Expected {338, 338}
    eclog::log_slice<eclog::circular_log, int> slice1_indexed = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), indexfile, 192, 1626771450, 1626772238, -128, 338);
    eclog::log_slice<eclog::circular_log, int> alt1 = log16.slice(1626771450, 1626772238);
    // Test timestamps out of bounds
    eclog::log_slice<eclog::circular_log, int> slice2 = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), NULL, 0, 1626771000, 1636779999, -128, 338);  // Expected {338, 338}
    eclog::log_slice<eclog::circular_log, int> slice2_indexed = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), indexfile, 192, 1626771000, 1636779999, -128, 338);
    eclog::log_slice<eclog::circular_log, int> alt2 = log16.slice(1626771000, 1636779999);
    // // Test random timestamps in different index entries
    eclog::log_slice<eclog::circular_log, int> slice3 = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), NULL, 0, 1626771508, 1626771990, -128, 338);  // Expected {374, 146}
    eclog::log_slice<eclog::circular_log, int> slice3_indexed = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), indexfile, 192, 1626771508, 1626771990, -128, 338);
    eclog::log_slice<eclog::circular_log, int> alt3 = log16.slice(1626771508, 1626771990);
    // Test exact entry timestamps
    eclog::log_slice<eclog::circular_log, int> slice4 = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), NULL, 0, 1626771560, 1626771720, -128, 338);  // Expected {422, 554}
    eclog::log_slice<eclog::circular_log, int> slice4_indexed = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), indexfile, 36, 1626771560, 1626771720, -128, 338);
    eclog::log_slice<eclog::circular_log, int> alt4 = log16.slice(1626771560, 1626771720);
    // Test random timestamps in same index entry
    eclog::log_slice<eclog::circular_log, int> slice5 = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), NULL, 0, 1626771981, 1626772022, -128, 338);  // Expected {134, 170}
    eclog::log_slice<eclog::circular_log, int> slice5_indexed = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), indexfile, 36, 1626771981, 1626772022, -128, 338);
    eclog::log_slice<eclog::circular_log, int> alt5 = log16.slice(1626771981, 1626772022);
    // // Test random timestamps in same log entry
    eclog::log_slice<eclog::circular_log, int> slice6 = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), NULL, 0, 1626772233, 1626772247, -128, 338);  // Expected {326, 338}
    eclog::log_slice<eclog::circular_log, int> slice6_indexed = eclog::make_log_slice<eclog::circular_log, int>(file, log16.get_file_size(), indexfile, 36, 1626772233, 1626772247, -128, 338);
    eclog::log_slice<eclog::circular_log, int> alt6 = log16.slice(1626772233, 1626772247);
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
    test11();
    test12();
    test13();
    test14();
    test15();
    test16();

    Success_Handler();
}
