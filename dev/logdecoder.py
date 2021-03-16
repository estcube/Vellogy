# Usage: python3 logdecoder.py path_to_binary_file T_format_string U_format_string TS_format_string
# Nt python3 data_logging/dev/logdecoder.py dump.bin I B Q

# Flush nõuaks suhtlust maa ja logimise vahel

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

deltas = []
data = []


def log_decode(buffer, T_format, U_format, TS_format):
    offset = len(buffer)
    data = []
    timestamps = []

    while offset > 0:
        formatstring = "<"

        offset -= struct.calcsize(U_format)
        data_added = struct.unpack_from("<" + U_format, buffer, offset)[0] + 1

        formatstring += TS_format
        for i in range(data_added):
            formatstring += T_format
            formatstring += U_format

        offset -= (struct.calcsize(U_format) + struct.calcsize(T_format)) * data_added + struct.calcsize(TS_format)
        entry = struct.unpack_from(formatstring, buffer, offset)

        timestamp = entry[0]
        for i in range(data_added * 2, 0, -2):
            timestamps.append(entry[i] + timestamp)
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
    T_format = sys.argv[2]
    U_format = sys.argv[3]  # NB! U_size can't be 0
    TS_format = sys.argv[4]

    # Read bytes from input binary file
    with open(sys.argv[1], "rb") as infile:
        log_bytes = bytes(infile.read())

    data, times = log_decode(log_bytes, T_format, U_format, TS_format)
    log_export_csv("log.csv", data, times)
    log_plot(data, times)
