#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
Wenqing Yan
This file is part of the MobiSys'23 Educational Backscatter Board Demo
18-May-2023
For the receiver board, read received data from CC2500 via serial port, compute and update the BER in real time
'''

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

import numpy as np
import pandas as pd
import datetime
import re
import random

global UPDATE_TIME, TX_RATE, PACKET_LEN, REF_PAYLOAD, PORT
UPDATE_TIME = 1 # unit is second
TX_RATE = 1 # every x second transmitter a packet is sent
PACKET_LEN = 32 # the payload length in bytes
REF_PAYLOAD = [15, 0, 47, 194, 57, 68, 211, 22, 125, 184, 183, 170, 1, 108, 219, 126] # the fixed payload content in decimal as a list
PORT = '/dev/ttyusb0'

ber_history = [0]
timeline  = [0]

delta = datetime.timedelta(seconds=UPDATE_TIME)

def validation_check(input_string):
    print_template = '\d{2}:\d{2}:\d{2}.\d{3}\s\|\s([0-9a-f]{2}\s){' + str(int(PACKET_LEN/2)) + '}\|\s-\d{2}\sCRC error'
    regex = re.compile(print_template, re.I)
    match = regex.match(str(input_string))
    return bool(match)

def popcount(n):
    return bin(n).count("1")

def compute_bit_errors(payload, sequence, PACKET_LEN=32):
    return sum(
        map(
            popcount,
            (
                np.array(payload[:PACKET_LEN])
                ^ np.array(sequence[: len(payload[:PACKET_LEN])])
            ),
        )
    )

def parse_payload(payload_string):
    tmp = map(lambda x: int(x, base=16), payload_string.split())
    return list(tmp)

def compute_ber(df, PACKET_LEN=32, TX_RATE=1, REF_PAYLOAD=[47, 194, 57, 68, 211, 22, 125, 184, 183, 170, 1, 108, 219, 126]):
    packets = len(df)
    # dataframe records the bit error for each packet
    error = pd.DataFrame(columns=['bit_error'], index=range(packets))
    # compute the number of packets supposed to been transmitted by the tag
    file_delay = df.timestamp[packets-1] - df.timestamp[0]
    file_delay = np.timedelta64(file_delay, "ms").astype(int) / 1000 # convert to seconds
    n_sent = file_delay * TX_RATE + 1
    # compute the number of missing packets
    n_missing = n_sent - packets
    # the file size
    file_size = n_sent * PACKET_LEN * 8

    # start count the error bits
    for idx in range(packets):
        # parse the payload to a list
        payload = parse_payload(df.payload[idx])
        error.bit_error[idx] = compute_bit_errors(payload, REF_PAYLOAD, PACKET_LEN)

    # total bit error counter initialization
    counter = sum(error.bit_error)
    counter += n_missing * PACKET_LEN * 8
    # print(file_delay, n_sent, n_missing, packets)
    # print(counter, file_size)

    return counter / file_size, error

def update(frame):
    df = pd.DataFrame(columns=['timestamp', 'payload', 'rss'])
    t_end = datetime.datetime.now() + delta
    # # open the port and read the serial output to dataframe
    # with serial.Serial(port, 115200)  as ser:
    #     while datetime.datetime.now() < t_end:
    #         rec = ser.read().decode("utf-8")
    #         if validation_check(line):
    #             tmp = line.split('|')
    #             time = datetime.datetime.strptime(tmp[0].rstrip(), "%H:%M:%S.%f")
    #             payload = tmp[1].strip()
    #             rss = int(tmp[2].split()[0])
    #             df = df.append({'timestamp': time, 'payload': payload, 'rss': rss}, ignore_index=True)

    # read from a file
    with open('./test.txt', 'r') as f:
        reader = f.readlines()
        for line in reader:
            if validation_check(line):
                tmp = line.split('|')
                time = datetime.datetime.strptime(tmp[0].rstrip(), "%H:%M:%S.%f")
                payload = tmp[1].strip()
                rss = int(tmp[2].split()[0])
                df = df.append({'timestamp': time, 'payload': payload, 'rss': rss}, ignore_index=True)
    print(df)
    file_ber, error = compute_ber(df, PACKET_LEN, TX_RATE, REF_PAYLOAD)
    file_ber += random.random()
    print(file_ber)
    ber_history.append(file_ber)
    timeline.append(timeline[-1] + 1)
    ln.set_data(timeline, ber_history)
    fig.gca().relim()
    fig.gca().autoscale_view()
    return ln

fig = plt.figure(figsize=(6, 3))
#creating a subplot
ax = fig.add_subplot(1,1,1)
ax.grid()
ax.set_xlabel('Time', fontsize=16)
ax.set_ylabel('Bit Error Rate [%]', fontsize=16)
ax.set_title('Real-time System Performance', fontsize=22)
ax.tick_params(labelsize=13)

ln, = plt.plot(timeline, ber_history, marker='o', markersize=6, color='black')
animation = FuncAnimation(fig, update, interval=(1000*UPDATE_TIME + 1000))
plt.show()
