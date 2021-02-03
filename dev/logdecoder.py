# Usage: python3 logdecoder.py path_to_binary_file size_of_T_in_bytes size_of_U_in_bytes size_of_TS_in_bytes

# Flush nõuaks suhtlust maa ja logimise vahel

import sys
import struct

T_size = 0
U_size = 0
TS_size = 0

log_bytes = []
decoder_position = 0

deltas = []
data = []

if __name__ == "__main__":
    T_size = int(sys.argv[2])
    U_size = int(sys.argv[3])  # NB! U_size can't be 0
    TS_size = int(sys.argv[4])

    # Read bytes from input binary file
    with open(sys.argv[1], "rb") as infile:
        log_bytes = bytes(infile.read())[::-1]

    while decoder_position < len(log_bytes):
        data_added = int.from_bytes(log_bytes[decoder_position:decoder_position+U_size], byteorder="big", signed=False) + 1
        decoder_position += U_size

        for i in range(data_added):
            delta = int.from_bytes(log_bytes[decoder_position:decoder_position+U_size], byteorder="big", signed=False)
            decoder_position += U_size
            datapoint = int.from_bytes(log_bytes[decoder_position:decoder_position+T_size], byteorder="big", signed=False)
            decoder_position += T_size

            deltas.append(delta)
            data.append(datapoint)

        timestamp = int.from_bytes(log_bytes[decoder_position:decoder_position+TS_size], byteorder="big", signed=False)
        decoder_position += TS_size

        for i in range(data_added):
            deltas[- 1 - i] += timestamp

    # For debug purposes
    print(deltas[::-1])
    print(data[::-1])
