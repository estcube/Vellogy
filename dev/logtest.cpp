#include <cmath>

#include "common.h"
#include "log.h"

#define PI 3.14159265

static void test1() {
    uint8_t file[1024];
    RegularLog<int> logi = RegularLog<int>(file, -3);
    Log log1 = Log(&logi);

    int datapoints[] = {12, 13, 14, 15, 2000, 3333, 4690};
    time_t timestamps[] = {1603723663, 1603723700, 1603723720, 1603724000, 1603724000, 1603724100, 1603724900};

    // Expected behaviour: first 3 datapoints are added to first entry
    // next 3 datapoints to the second entry
    // last datapoint is added to the third entry
    for (uint8_t i = 0; i < 7; i++) {
        log1.log(datapoints[i], timestamps[i]);
    }
}

static void test2() {
    double result[360];
    time_t timestamp = 1603723663;

    uint8_t file[1024];
    RegularLog<double> logi = RegularLog<double>(file, -3);
    Log log2 = Log(&logi);

    // Save sine data (to be plotted later)
    for (uint16_t time = 0; time < 360; time++) {
        result[time] = sin((timestamp + time) * PI / 180.0);
        log2.log(result[time], timestamp + time);
    }
}

static void test3() {
    // Metafile containing random decode info
    uint8_t metafile[] = {
        // Decode info (random)
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // File entries
        0x00, 0x00, 0x00, 0x00,
        // File size
        0x00, 0x00, 0x00, 0x00,
        // Indexfile size
        0x00, 0x00, 0x00, 0x00
    };

    uint8_t indexfile[128];
    uint8_t file[1024];

    RegularLog<int> logi = RegularLog<int>(metafile, indexfile, file, -3);
    Log log3 = Log(&logi);

    int datapoints[] = {14, 15, 2000, 3333, 4690};
    time_t timestamps[] = {1603723720, 1603724000, 1603724000, 1603724100, 1603724900};

    for (uint8_t i = 0; i < 5; i++) {
        log3.log(datapoints[i], timestamps[i]);
    }

    log3.save_meta_info();
}

static void test4() {
    uint8_t file[1024];
    RegularLog<int> logi = RegularLog<int>(file, -3);
    Log log4 = Log(&logi);

    int data[100];
    time_t timestamp = 1603723663;

    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        log4.log(data[i], timestamp + i*64);
    }
}

static void test5() {
    // Test restoring log from existing metafile, indexfile and file

    // Metafile containing random decode info
    uint8_t metafile[200] = {
        // decode info
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // number of file entries
        0x18, 0x00, 0x00, 0x00,
        // file size in bytes
        0xb8, 0x02, 0x00, 0x00,
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
    RegularLog<int> logi = RegularLog<int>(metafile, indexfile, file, -3);
    Log log5 = Log(&logi);

    int data[100];
    time_t timestamp = 1603730000;

    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        log5.log(data[i], timestamp + i*64);
    }

    log5.save_meta_info();

    // A log to test whether metainfo was serialized correctly
    RegularLog<int> logi_test = RegularLog<int>(metafile, indexfile, file, -3);
}

static void test6() {
    // Test log slicing

    uint8_t metafile[200] = {
        // decode info (random)
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // number of file entries
        0x00, 0x00, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // index file size in bytes
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t indexfile[128];
    uint8_t file[1024];

    RegularLog<int> logi = RegularLog<int>(metafile, indexfile, file, -3);
    Log log6 = Log(&logi);

    int data[101];
    time_t timestamp = 1603730000;

    // Create a log file with 25 entries, each entry with 4 datapoints (size of one full log entry is 29 bytes)
    // One extra datapoint shall remain in volatile memory
    for (uint8_t i = 0; i < 101; i++) {
        data[i] = i*100;
        log6.log(data[i], timestamp + i*64);
    }

    log6.save_meta_info();

    // A log to test whether metainfo was serialized correctly
    RegularLog<int> logi_test = RegularLog<int>(metafile, indexfile, file, -3);

    // Exact timestamp tests:
    // Entries in different log and index entries
    LogSlice<RegularLog, int> slice1 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730064, 1603735056, -3);  // Expected {0, 580}
    // Entries in different log, but same index entries
    LogSlice<RegularLog, int> slice2 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730320, 1603730576, -3);  // Expected {29, 87}
    // Entries in the same log (and index) entry
    LogSlice<RegularLog, int> slice3 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730064, 1603730128, -3);  // Expected {0, 29}

    // Random timestamp tests:
    // Entries in different log and index entries
    LogSlice<RegularLog, int> slice4 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603732103, 1603735077, -3);  // Expected {232, 580}
    LogSlice<RegularLog, int> alt4 = log6.slice(1603732103, 1603735077);
    // Entries in different log, but same index entries
    LogSlice<RegularLog, int> slice5 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730261, 1603730619, -3);  // Expected {29, 87}
    LogSlice<RegularLog, int> alt5 = log6.slice(1603730261, 1603730619);
    // A big slice
    LogSlice<RegularLog, int> slice6 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730901, 1603736130, -3);  // Expected {87, 696}
    LogSlice<RegularLog, int> alt6 = log6.slice(1603730901, 1603736130);
    // Entries in consecutive log entries
    LogSlice<RegularLog, int> slice7 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603734070, 1603734098, -3);  // Expected {435, 493}
    LogSlice<RegularLog, int> alt7 = log6.slice(1603734070, 1603734098);

    // Edge cases
    // Both timestamps in the very last entry of the log file
    LogSlice<RegularLog, int> slice8 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603736170, 1603736330, -3);  // Expected {696, 725}
    // Timestamps are entry timestamps
    LogSlice<RegularLog, int> slice9 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730512, 1603736144, -3);  // Expected {58, 725}
    // Timestamps are entry timestamps
    LogSlice<RegularLog, int> slice10 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603736144, 1603736336, -3);  // Expected {696, 725}
    // Timestamps are index entry timestamps
    LogSlice<RegularLog, int> slice11 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603732560, 1603735120, -3);  // Expected {290, 609}
    // End timestamp conincides with middle timestamp of the index
    LogSlice<RegularLog, int> slice12 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730512, 1603732560, -3);  // Expected {58, 319}
    // Index only contains one (full) entry
    LogSlice<RegularLog, int> slice13 = log_slice<RegularLog, int>(file, 29 * 5, indexfile, 1 * (sizeof(time_t) + sizeof(uint32_t)), 1603730011, 1603730200, -3);  // Expected {0, 29}
    // Index only contains one (half) entry
    LogSlice<RegularLog, int> slice14 = log_slice<RegularLog, int>(file, 29 * 3, indexfile, 1 * (sizeof(time_t) + sizeof(uint32_t)), 1603730003, 1603730030, -3);  // Expected {0, 29}

    // (Semi-)bad/pointless things the user can do
    // Boundary timestamps out of bounds
    LogSlice<RegularLog, int> slice15 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 10, 2002002002, -3);  // Expected {0, 725}
    // End timestamp smaller than start timestamp
    LogSlice<RegularLog, int> slice16 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 2002002002, 10, -3);  // Expected {0, 725}
    // End timestamp equal to start timestamp
    LogSlice<RegularLog, int> slice17 = log_slice<RegularLog, int>(file, logi.get_file_size(), indexfile, 5 * (sizeof(time_t) + sizeof(uint32_t)), 1603730512, 1603730512, -3);  // Expected {58, 87}

    uint8_t new_file[512];
    Log<RegularLog, int> new_log = slice4.createLog(new_file);
}

static void test7() {
    // Test repeating timestamps

    uint8_t metafile[200] = {
        // decode info (random)
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // number of file entries
        0x00, 0x00, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // index file size in bytes
        0x00, 0x00, 0x00, 0x00
    };
    uint8_t indexfile[128];
    uint8_t file[1536];
    RegularLog<int> logi = RegularLog<int>(metafile, indexfile, file, -3);
    Log log7 = Log(&logi);

    int data[300];
    time_t timestamp = 1603723663;

    // Test an edge case where all the datapoints have the same timestamp
    for (uint16_t i = 0; i < 300; i++) {
        data[i] = i*100;
        log7.log(data[i], timestamp);
    }
}

static void test8() {
    // Test optionality of indexfile

    uint8_t file[1024];
    RegularLog<int> logi = RegularLog<int>(file, -3);  // Special constructor
    Log log8 = Log(&logi);

    int data[100];
    time_t timestamp = 1603730000;

    // No index entries should be created after this loop
    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        log8.log(data[i], timestamp + i*64);
    }

    // Should have no effect
    log8.save_meta_info();

    // Should work as expected
    LogSlice<RegularLog, int> slice1 = log_slice<RegularLog, int>(file, logi.get_file_size(), NULL, 0, 1603732103, 1603735077, -3);  // Expected {232, 580}
    LogSlice<RegularLog, int> slice2 = log_slice<RegularLog, int>(file, logi.get_file_size(), NULL, 0, 1603730261, 1603730619, -3);  // Expected {29, 87}
    LogSlice<RegularLog, int> slice3 = log_slice<RegularLog, int>(file, logi.get_file_size(), NULL, 0, 1603730901, 1603736130, -3);  // Expected {87, 696}
    LogSlice<RegularLog, int> slice4 = log_slice<RegularLog, int>(file, logi.get_file_size(), NULL, 0, 1603734070, 1603734098, -3);  // Expected {435, 493}
}

static void test9() {
    // Test flushing log when entry is not yet full

    uint8_t file[1024];
    RegularLog<int> logi = RegularLog<int>(file, -3);
    Log log9 = Log(&logi);

    int data[100];
    time_t timestamp = 1603723700;

    for (uint16_t i = 0; i < 50; i++) {
        data[i] = i*100;
        log9.log(data[i], timestamp + i * 64);
    }

    // After this, a new entry with only 2 datapoints should be written to log file
    log9.flush();

    // After this loop, 12 new entries should be created and volatile memory should hold 2 datapoints
    for (uint16_t i = 50; i < 100; i++) {
        data[i] = i*100;
        log9.log(data[i], timestamp + i * 64);
    }
}

static void test10() {
    // Test different resolutions for log

    uint8_t file1[1024];
    RegularLog<int> logi1 = RegularLog<int>(file1, -3);
    Log log10a = Log(&logi1);

    int data[100];
    time_t timestamp = 1603723700;

    // Expected behaviour: all timedeltas are logged in their original form
    for (uint16_t i = 0; i < 50; i++) {
        data[i] = i*100;
        log10a.log(data[i], timestamp + i * 4);
    }

    uint8_t file2[1024];
    RegularLog<int> logi2 = RegularLog<int>(file2, -2);
    Log log10b = Log(&logi2);

    // Expected behaviour: timedeltas are 0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6, 6, ...
    // (these are the rounded tens of the actual timedeltas)
    for (uint16_t i = 0; i < 50; i++) {
        data[i] = i*100;
        log10b.log(data[i], timestamp + i * 4);
    }
    // Since timedeltas are compressed, the entry won't be full after this loop, so we need to end it manually to see changes in file
    log10b.flush();

    uint8_t file3[1024];
    RegularLog<int> logi3 = RegularLog<int>(file3, -1);
    Log log10c = Log(&logi3);

    // Expected behaviour: timedeltas are 0 (4 times), 1 (6 times), 2 (6 times) and so on
    // (these are the rounded hundreds of the actual timedeltas)
    for (uint16_t i = 0; i < 50; i++) {
        data[i] = i*100;
        log10c.log(data[i], timestamp + i * 16);
    }
    // Since timedeltas are compressed, the entry won't be full after this loop, so we need to end it manually to see changes in file
    log10c.flush();

    uint8_t file4[1024];
    RegularLog<int> logi4 = RegularLog<int>(file4, 0);
    Log log10d = Log(&logi4);

    // Expected behaviour: timedeltas are 0 (2 times), 1 (4 times), 2 (4 times), 3 (4 times) and so on
    // (these are the rounded thousands of the actual timedeltas)
    for (uint16_t i = 0; i < 50; i++) {
        data[i] = i*100;
        log10d.log(data[i], timestamp + i * 256);
    }
    // Since timedeltas are compressed, the entry won't be full after this loop, so we need to end it manually to see changes in file
    log10d.flush();
}

static void test11() {
    // Test SimpleLog functionality

    // First with indexfile
    // Metafile containing random decode info
    uint8_t metafile[200] = {
        // decode info
        0x00, 0x00, 0x00, 0x10, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x00, 0x00,
        // number of file entries
        0x00, 0x00, 0x00, 0x00,
        // file size in bytes
        0x00, 0x00, 0x00, 0x00,
        // indexfile size in bytes
        0x00, 0x00, 0x00, 0x00,
    };
    uint8_t indexfile[256];
    uint8_t file[2048];

    SimpleLog<int> logi = SimpleLog<int>(metafile, indexfile, file);
    Log log11 = Log(&logi);

    int data[100];
    time_t timestamp = 1603740000;

    // Expected behaviour: the log file is 1200 bytes larger after this loop and 20 index entries are created
    for (uint8_t i = 0; i < 100; i++) {
        data[i] = i*100;
        log11.log(data[i], timestamp + i*64);
    }

    // Test whole file slice
    LogSlice<SimpleLog, int> slice1 = log_slice<SimpleLog, int>(file, logi.get_file_size(), NULL, 0, 1603740000, 1603746400, -3);  // Expected {0, 1200}
    LogSlice<SimpleLog, int> alt1 = log11.slice(1603740000, 1603746400);
    // Test timestamps out of bounds
    LogSlice<SimpleLog, int> slice2 = log_slice<SimpleLog, int>(file, logi.get_file_size(), NULL, 0, 1603730000, 1603766400, -3);  // Expected {0, 1200}
    LogSlice<SimpleLog, int> alt2 = log11.slice(1603730000, 1603766400);
    // Test random timestamps in different index entries
    LogSlice<SimpleLog, int> slice3 = log_slice<SimpleLog, int>(file, logi.get_file_size(), NULL, 0, 1603740131, 1603744681, -3);  // Expected {24, 888}
    LogSlice<SimpleLog, int> alt3 = log11.slice(1603740131, 1603744681);
    // Test exact entry timestamps
    LogSlice<SimpleLog, int> slice4 = log_slice<SimpleLog, int>(file, logi.get_file_size(), NULL, 0, 1603740640, 1603742560, -3);  // Expected {120, 492}
    LogSlice<SimpleLog, int> alt4 = log11.slice(1603740640, 1603742560);
    // Test random timestamps in same index entry
    LogSlice<SimpleLog, int> slice5 = log_slice<SimpleLog, int>(file, logi.get_file_size(), NULL, 0, 1603740972, 1603741099, -3);  // Expected {180, 216}
    LogSlice<SimpleLog, int> alt5 = log11.slice(1603740972, 1603741099);
    // Test random timestamps in same log entry
    LogSlice<SimpleLog, int> slice6 = log_slice<SimpleLog, int>(file, logi.get_file_size(), NULL, 0, 1603740972, 1603740980, -3);  // Expected {180, 192}
    LogSlice<SimpleLog, int> alt6 = log11.slice(1603740972, 1603740980);

    uint8_t new_file[512];
    Log<SimpleLog, int> new_log = alt5.createLog(new_file);
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

    Success_Handler();
}
