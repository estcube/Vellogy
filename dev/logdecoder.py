# Usage for decoding regular/simple logs:
# python3 logdecoder.py path_to_binary_file log_type T_format_string TS_format_string TS_subseconds_base TS_subseconds_resolution TS_subseconds_resolution_in_log
# Last 3 parameters are not needed for simple logs
# Example 1 (regular log): python3 data_logging/dev/logdecoder.py dump.bin regular I Q 10 -3 -1
# Example 2 (simple log): python3 data_logging/dev/logdecoder.py dump.bin simple I Q

import sys
import struct
import csv
import matplotlib.pyplot as plt

c_types = {
    "int8_t" : "b",
    "uint8_t" : "B",
    "int16_t" : "h",
    "uint16_t" : "H",
    "int32_t" : "i",
    "uint32_t" : "I",
    "float" : "f",
    "double" : "d"
}

c_type_sizes = {
    "int8_t" : 1,
    "uint8_t" : 1,
    "int16_t" : 2,
    "uint16_t" : 2,
    "int32_t" : 4,
    "uint32_t" : 4,
    "float" : 4,
    "double" : 8
}

T_format = ""
U_format = ""
TS_format = ""

log_bytes = []


def simplelog_decode(buffer, T_format, TS_format):
    data = []
    timestamps = []

    # Note: "<" means little-endian
    formatstring = "<" + TS_format + T_format
    entry_size = struct.calcsize(TS_format) + struct.calcsize(T_format)

    offset = 0
    while offset < len(buffer):
        # Unpack one entry (timestamp + datapoint) at a time
        entry = struct.unpack_from(formatstring, buffer, offset)
        offset += entry_size

        timestamps.append(entry[0])
        data.append(entry[1])

    return data, timestamps


def regularlog_decode(buffer, T_format, TS_format, rel_precision):
    data = []
    timestamps = []

    U_format = "B"

    offset = len(buffer)
    while offset > 0:
        # Note: "<" means little-endian
        formatstring = "<"

        offset -= struct.calcsize(U_format)
        # Data added is always one more than recorded in the log file
        data_added = struct.unpack_from("<" + U_format, buffer, offset)[0] + 1

        # Contruct the formatstring from timestamp, data and timedelta formats in the order they appear in the (regular) log file
        formatstring += TS_format
        for i in range(data_added):
            formatstring += T_format
            formatstring += U_format

        # Move the buffer pointer into the beginning of the entry
        offset -= (struct.calcsize(U_format) + struct.calcsize(T_format)) * data_added + struct.calcsize(TS_format)
        # Read in the entry (everything except data_added), using the constructed formatstring
        entry = struct.unpack_from(formatstring, buffer, offset)

        timestamp = entry[0]
        # Start adding datapoints and timestamps into the decoded log, starting from the end
        for i in range(data_added * 2, 0, -2):
            # Scale logged timedeltas back into right timestamp resolution and add the entry timestamp to them
            timestamps.append(entry[i] * rel_precision + timestamp)
            data.append(entry[i - 1])

    return data[::-1], timestamps[::-1]


def log_export_csv(filename, data, timestamps):
    with open(filename, 'w', newline='\n') as file:
        writer = csv.writer(file, delimiter=',')
        for i in range(len(data)):
            writer.writerow([times[i], data[i]])


def log_plot(data, timestamps):
    plt.plot(timestamps, data)
    plt.show()


if __name__ == "__main__":
    log_type = sys.argv[2]
    T_format = sys.argv[3]
    TS_format = sys.argv[4]

    # Read bytes from input binary file
    with open(sys.argv[1], "rb") as infile:
        log_bytes = bytes(infile.read())

    # Invoke different functions for decoding based on log type
    if log_type == "regular":
        # How much less precise are the logged timedeltas from the actual timestamps
        rel_precision = int(sys.argv[5]) ** abs(int(sys.argv[6]) - int(sys.argv[7]))
        data, times = regularlog_decode(log_bytes, T_format, TS_format, rel_precision)
    elif log_type == "simple":
        data, times = simplelog_decode(log_bytes, T_format, TS_format)

    log_export_csv("log.csv", data, times)
    log_plot(data, times)
