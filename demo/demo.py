#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
Wenqing Yan
This file is part of the MobiSys'23 Educational Backscatter Board Demo
18-May-2023
For the receiver board, read received data from CC2500 via serial port, compute and update the BER in real time
'''

'''
Todo:
BER calculation
add the seq number
consider the bit error in length field
'''

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

import numpy as np
import pandas as pd
import datetime
import re
import serial
from functools import partial
import random

global UPDATE_TIME, TX_RATE, PORT
UPDATE_TIME = 1 # unit is second
PACKET_LEN = 15 # the payload length in bytes
PORT = '/dev/tty.usbmodem101'
REF_PAYLOAD = [0xa5]
delta = datetime.timedelta(seconds=UPDATE_TIME)

def validation_check(input_string):
    print_template = '\d{2}:\d{2}:\d{2}.\d{3}\s\|\s([0-9a-f]{2}\s){1,256}\|\s-\d{2}\sCRC error'
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

def compute_ber(df, PACKET_LEN=15, REF_PAYLOAD=[0xa5]):
    packets = len(df)
    first_seq = parse_payload(df.payload[0])[1]
    last_seq = parse_payload(df.payload[packets-1])[1]
    file_size = (last_seq-first_seq+1) * (PACKET_LEN-1) * 8
    # dataframe records the bit error for each packet
    error = pd.DataFrame(columns=['bit_error'], index=range(first_seq, last_seq+1))
    # bit_errors list initialization
    error.bit_error = [list() for x in range(len(error))]

    # start count the error bits
    for idx in range(packets):
        # parse the payload to a list
        payload = parse_payload(df.payload[idx])
        # deal with packet length error
        if payload[0] >= (PACKET_LEN):
            payload = payload[:PACKET_LEN+1]
        else:
            payload = payload + [0]*(PACKET_LEN-payload[0])
        ## dealing with missing packet
        error.bit_error[payload[1]].append(compute_bit_errors(payload[2:], REF_PAYLOAD * (PACKET_LEN-1), PACKET_LEN-1))

    # total bit error counter initialization
    counter = 0
    n_missing = 0
    for l in error.bit_error:
        if l == []:
            n_missing += 1
        elif len(l) > 1:
            n_missing -= 1
        else:
            tmp =  min(l)
        counter += sum(l)
    counter += n_missing * (PACKET_LEN-1)
    return counter / file_size, error


def update(frame, fig, art, timeline, ber_history, ser, start_timestamp):
    # open the port
    ser.open()
    df = pd.DataFrame(columns=['timestamp', 'payload', 'rss'])
    t_end = datetime.datetime.now() + delta
    # config the receiver clicker board the serial port
    config = pd.read_excel('config.xlsx')
    command = f"c {config.center[0]} {config.deviation[0]} {config.baud[0]} {config.bandwidth[0]}\r"
    ser.write(command.encode('utf-8'))
    # read the serial output
    while datetime.datetime.now() < t_end:
        rec = ser.readline().decode("utf-8")
        print(rec)
        if validation_check(rec):
            tmp = rec.split('|')
            time = datetime.datetime.strptime(tmp[0].rstrip(), "%H:%M:%S.%f")
            payload = tmp[1].strip()
            rss = int(tmp[2].split()[0])
            df = df.append({'timestamp': time, 'payload': payload, 'rss': rss}, ignore_index=True)
    # stop the reception
    ser.write("t\r".encode('utf-8'))
    # close the port
    ser.close()

    # analyse the received data
    if len(df) >= 1:
        file_ber, error = compute_ber(df, PACKET_LEN, REF_PAYLOAD)
        print(file_ber)
    else:
        file_ber = 1
    # update the figure
    ber_history.append(file_ber)
    time_delta = t_end-start_timestamp
    print(time_delta.seconds)
    timeline.append(time_delta.seconds)
    # only show last N data points
    if len(timeline) > 10:
        timeline = timeline[-10:]
        ber_history = ber_history[-10:]
    art.set_data(timeline, ber_history)
    fig.gca().relim()
    fig.gca().autoscale_view()
    return art

def update_test(frame, fig, art, timeline, ber_history, start_timestamp):
    df = pd.DataFrame(columns=['timestamp', 'payload', 'rss'])
    t_end = datetime.datetime.now() + delta
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
    file_ber, error = compute_ber(df, PACKET_LEN, REF_PAYLOAD)
    # file_ber += random.random()
    ber_history.append(file_ber)
    time_delta = t_end-start_timestamp
    print(f"Timestamp:{time_delta.seconds}s, BER:{file_ber*100:.2f}%\n")
    timeline.append(time_delta.seconds)
    # only show last N data points
    if len(timeline) > 10:
        timeline = timeline[-10:]
        ber_history = ber_history[-10:]
    art.set_data(timeline, ber_history)
    fig.gca().relim()
    fig.gca().autoscale_view()
    return art

# main fuction
def main():
    ber_history = []
    timeline  = []
    with serial.Serial(PORT, 115200) as ser:
        fig = plt.figure(figsize=(6, 3))
        ax = fig.add_subplot(1,1,1)
        # configure the figure display
        ax.grid()
        ax.set_xlabel('Time [s]', fontsize=16)
        ax.set_ylabel('Bit Error Rate [%]', fontsize=16)
        ax.set_title('Real-time System Performance', fontsize=22)
        ax.tick_params(labelsize=13)

        ln, = plt.plot(timeline, ber_history, marker='o', markersize=6, color='black')
        start_timestamp = datetime.datetime.now()
        animation = FuncAnimation(fig, partial(update, fig=fig, art=ln, timeline=timeline, ber_history=ber_history, ser=ser, start_timestamp=start_timestamp), interval=(1000*UPDATE_TIME + 1000))
    # animation = FuncAnimation(fig, partial(update_test, fig=fig, art=ln, timeline=timeline, ber_history=ber_history, start_timestamp=start_timestamp), interval=(1000*UPDATE_TIME + 1000))
    plt.show()

if __name__ == '__main__':
    main()
