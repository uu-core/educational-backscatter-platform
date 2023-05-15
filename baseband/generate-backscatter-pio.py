#!/usr/bin/python3

# Tobias Mages and Wenqing Yan
# Date:   06.03.2023
# Course: Wireless Communication and Networked Embedded Systems, Project VT2023
# Generate PIO state-machine for specific radio seetings
#

# usage example: python generate-backscatter-pio.py --help
# usage example: python generate-backscatter-pio.py 20 18 100000 ./backscatter.pio
# usage example: python generate-backscatter-pio.py 20 18 100000 ./backscatter.pio --twoAntennas

import argparse
from pathlib import Path
CLKFREQ = 125 # MHz

# parse arguments and give help option
parser = argparse.ArgumentParser(prog = 'Backscatter-PIO generator', description='Wireless Communication and Networked Embedded Systems, Project VT2023\nusage example: python3 generate-backscatter-pio.py 20 18 100000 ./backscatter.pio')
parser.add_argument('d0', type=int, help=f'clock divider @ {CLKFREQ} MHz for frequency 0 shift; must be an even number e.g. 20 for {(CLKFREQ/20):.3f} MHz')
parser.add_argument('d1', type=int, help=f'clock divider @ {CLKFREQ} MHz for frequency 1 shift; must be an even number e.g. 18 for {(CLKFREQ/18):.3f} Mhz')
parser.add_argument( 'b', type=int, help='baud-rate [baud] e.g. 100000 for 100kBaud')
parser.add_argument( 'f', type=str, help='output path/file-name')
parser.add_argument('--twoAntennas', help='if used, generates PIO for transmission on two antennas (in-phase)', action='store_true')
args = parser.parse_args()

d0 = args.d0
d1 = args.d1
b = args.b
if (CLKFREQ*(10**6)) % args.b != 0:
    b = round((CLKFREQ*(10**6)) / round((CLKFREQ*(10**6)) / args.b))
    print(f'\nWARNING: a baudrate of {args.b} Baud is not achievable with a {CLKFREQ} MHz clock.\nTherefore, the closest achievable baud-rate {b} Baud will be used.\n')
out_path = Path(args.f)
TWOANTENNAS = args.twoAntennas

assert d0 % 2 == 0 and d0 >= 2, 'd0 must be an even integer larger than 1'
assert d1 % 2 == 0 and d1 >= 2, 'd1 must be an even integer larger than 1'
assert b > 0, 'baud-rate can not be negative'
splitNbit = lambda x,n: [(2**n) for i in range(0,x//(2**n))] + ([(x % (2**n))] if (x % (2**n)) != 0 else [])
split5bit = lambda x: splitNbit(x,5)
split3bit = lambda x: splitNbit(x,3)
sleeptime5bit = lambda x,subLast=0: [s-1 for s in lastMinus(split5bit(x),subLast)]
sleeptime3bit = lambda x,subLast=0: [s-1 for s in lastMinus(split3bit(x),subLast)]
sleeptime  = sleeptime3bit if TWOANTENNAS else sleeptime5bit
splitDelay = split3bit if TWOANTENNAS else split5bit
lastMinus = lambda l,x: l[:-1]+([l[-1]-x] if l[-1]-x > 0 else [])
fcenter = (CLKFREQ*1000/d0 + CLKFREQ*1000/d1)/2
fdeviation = abs(CLKFREQ*1000/d1 - fcenter)
lastPeriodCycles1 = (CLKFREQ*1000000//b - 4) % d1
lastPeriodCycles0 = (CLKFREQ*1000000//b - 4) % d0

# generate pio-file
pio_file = '\n'.join([f';', '; Automatically generated using "generate-backscatter-pio.py"',
f'; with the command: "python generate-backscatter-pio.py {d0} {d1} {b} {out_path} {("--twoAntennas" if TWOANTENNAS else "")}"',';',
 '; Backscatter PIO', ('; Configured for two antenns' if TWOANTENNAS else '; Configured for one antenna'), ';' ,'', '.program backscatter'] + (['.side_set 1 opt'] if TWOANTENNAS else []) + ['',
 '; --- PIO settings ---',
 '; configer autopull',
f'; configered for {CLKFREQ} MHz clock', '',
 '; --- backscatter settings ---',
f'; frequency 0 shift: {(CLKFREQ/d0):.3f} MHz       (1 period = {d0} cycles @ {CLKFREQ} MHz clock)',
f'; frequency 1 shift: {(CLKFREQ/d1):.3f} Mhz       (1 period = {d1} cycles @ {CLKFREQ} MHz clock)',
f'; center frequency shift: {((CLKFREQ/d0 + CLKFREQ/d1)/2):.3f} MHz',
f'; deviation from center : {fdeviation:.2f} kHz',
f'; baud-rate {(b/1000):.2f} kBaud ({(CLKFREQ*1000000/b):.1f} instructions per symbol)',
f'; occupied bandwith: {(b/1000 + 2*fdeviation):.2f} kHz',''] +
(['; WARNING: the deviation is too large for the CC2500'] if (fdeviation > 380.86) else []) +
(['; WARNING: the deviation is too large for the CC1352'] if (fdeviation > 1000) else []) +
(['; WARNING: symbol 0 has been assigned to larger frequncy than symbol 1'] if (d0 < d1) else []) + ['',
f'; parameter 1:  b = clock-frequency/baud-rate (e.g. {(CLKFREQ*1000000/b):.1f} for {b/1000:.2f} kBaud @ {CLKFREQ} MHz clock)   // number of clock cycles per symbol',
 '; parameter 2:  w = 4 (wasted cycles per symbol: OUT -> JMP -> MOV -> ... -> JMP )            // fixed wasted cycles per symbol',
f'; parameter 3: d0 = clock-frequency/shift-frequency-0 (e.g. {d0} for {CLKFREQ/d0:.3f} MHz @ {CLKFREQ} MHz clock) // must be an _even_ number',
f'; parameter 4: d1 = clock-frequency/shift-frequency-1 (e.g. {d1} for {CLKFREQ/d1:.3f} Mhz @ {CLKFREQ} MHz clock) // must be an _even_ number', '', '',
 '; interface: ',
 '; comment: the parameters have to be obtained from the fifo, since the SET command only provides 5-bit',
 '; 1. obtain from fifo: floor((b - w) / d0) - 1 // (number of full periods in symbol of frequency 0)',
 '; 2. obtain from fifo: floor((b - w) / d1) - 1 // (number of full periods in symbol of frequency 1)',
 '; 3. then simply provide data', '',
f'SET  pins   1  {("side 1" if TWOANTENNAS else "      ")}  ; swtich output off',
 'OUT   isr  32          ; baud config for frequency 0',
 'OUT     y  32          ; baud config for frequency 1',
 'get_symbol:',
 '    OUT  x  1          ; get data bit',
 '    JMP !x  send_0     ; jmp if x is zero',
 '    send_1:',
 '        MOV x  y                       ; load baud 1 config',
 '        loop_1:'] +
[f'            SET pins 1  {("side 1" if TWOANTENNAS else "      ")}  [{x}]    ; for {(CLKFREQ/d1*1000):.1f} kHz - {d1//2} cycles high' for x in sleeptime(d1//2)] +
[f'            SET pins 0  {("side 0" if TWOANTENNAS else "      ")}  [{x}]    ; for {(CLKFREQ/d1*1000):.1f} kHz - {d1//2} cycles low' for x in sleeptime(d1//2,1)] + [
 '            JMP x-- loop_1             ; 1 cycle  ',
 '        ; to avoid a drift from imprecise baud-timing: stop the last period on time',
f'        ; the remaining cycles are:  (b - w) % d1 = ({(CLKFREQ*1000000//b)} - 4) % {d1} => {lastPeriodCycles1} cycles left to spend '] +
([f'        SET pins 1  {("side 1" if TWOANTENNAS else "      ")}  [{x-1}]        ; spend {min([(lastPeriodCycles1),d1//2])} cycles of last period on high' for x in splitDelay(min([(lastPeriodCycles1),d1//2]))] if lastPeriodCycles1 > 0 else []) +
([f'        SET pins 0  {("side 0" if TWOANTENNAS else "      ")}  [{x-1}]        ; spend {lastPeriodCycles1 - d1//2} cycles of last period on low' for x in splitDelay(lastPeriodCycles1 - d1//2)] if lastPeriodCycles1 - d1//2 > 0 else []) + [
 '        JMP get_symbol                 ; ',
 '    send_0:',
 '        MOV x isr                      ; load baud 0 config',
 '        loop_0:'] +
[f'            SET pins 1  {("side 1" if TWOANTENNAS else "      ")}  [{x}]    ; for {(CLKFREQ/d0*1000):.1f} kHz - {d0//2} cycles high' for x in sleeptime(d0//2)] +
[f'            SET pins 0  {("side 0" if TWOANTENNAS else "      ")}  [{x}]    ; for {(CLKFREQ/d0*1000):.1f} kHz - {d0//2} cycles low' for x in sleeptime(d0//2,1)] + [
 '            JMP x-- loop_0             ; 1 cycle  ',
 '        ; to avoid a drift from imprecise baud-timing: stop the last period on time',
f'        ; the remaining cycles are:  (b - w) % d0 = ({(CLKFREQ*1000000//b)} - 4) % {d0} => {lastPeriodCycles0} cycles left to spend '] +
([f'        SET pins 1  {("side 1" if TWOANTENNAS else "      ")}  [{x-1}]        ; spend {min([(lastPeriodCycles0),d0//2])} cycles of last period on high' for x in splitDelay(min([(lastPeriodCycles0),d0//2]))] if lastPeriodCycles0 > 0 else []) +
([f'        SET pins 0  {("side 0" if TWOANTENNAS else "      ")}  [{x-1}]        ; spend {lastPeriodCycles0 - d0//2} cycles of last period on low' for x in splitDelay(lastPeriodCycles0 - d0//2)] if lastPeriodCycles0 - d0//2 > 0 else []) + [
 '        JMP get_symbol                 ; ', '','% c-sdk {',
 '#include "pico/stdlib.h"',
 '#include "hardware/clocks.h"',
 '#define min(x, y) (((x) < (y)) ? (x) : (y))',
f'#define PIO_BAUDRATE {b}',
f'#define PIO_CENTER_OFFSET {round(fcenter*1000)}',
f'#define PIO_DEVIATION {round(fdeviation*1000)}',
f'#define PIO_MIN_RX_BW {round((b/1000 + 2*fdeviation)*1000)}', '',
f'static inline void backscatter_program_init(PIO pio, uint sm, uint offset, uint pin1{", uint pin2" if TWOANTENNAS else ""})' + '{',
 '   pio_gpio_init(pio, pin1);',
 '   pio_sm_set_consecutive_pindirs(pio, sm, pin1, 1, true);'] + ([
 '   pio_gpio_init(pio, pin2);',
 '   pio_sm_set_consecutive_pindirs(pio, sm, pin2, 1, true);'] if TWOANTENNAS else []) + [
 '   pio_sm_config c = backscatter_program_get_default_config(offset);',
 '   sm_config_set_set_pins(&c, pin1, 1);']  + ([
 '   sm_config_set_sideset_pins(&c, pin2);'] if TWOANTENNAS else []) + [
 '   sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX); // We only need TX, so get an 8-deep FIFO (join RX and TX FIFO)',
 '   sm_config_set_out_shift(&c, false, true, 32);  // OUT shifts to left (MSB first), autopull after every 32 bit',
 '   pio_sm_init(pio, sm, offset, &c);',
 '   pio_sm_set_enabled(pio, sm, true);',
f'   pio_sm_put_blocking(pio, sm, {((CLKFREQ*1000000//b - 4) // d0) - 1}); // floor((b - w) / d0) - 1 = floor(({CLKFREQ*1000000//b} - 4)/{d0}) - 1   // -1 is requried since JMP 0-- is still true',
f'   pio_sm_put_blocking(pio, sm, {((CLKFREQ*1000000//b - 4) // d1) - 1}); // floor((b - w) / d1) - 1 = floor(({CLKFREQ*1000000//b} - 4)/{d1}) - 1   // -1 is required since JMP 0-- is still true',
 '}', '', '',
 'static inline void backscatter_send(PIO pio, uint sm, uint32_t *message, uint32_t len) {',
 '    for(uint32_t i = 0; i < len; i++){',
 '        pio_sm_put_blocking(pio, sm, message[i]); // set pin back to low',
 '    }',
f'    sleep_ms(1); // wait for transmission to finish', '}', '','%}'])

# write pio-file
with open(out_path, 'w') as out_file:
    out_file.write(pio_file)

# print radio settings and warnings
print('\nGenerated Radio seetings:\n' + '\n'.join([f'  - frequency 0 shift: {(CLKFREQ/d0):.3f} MHz       (1 period = {d0} cycles @ {CLKFREQ} MHz clock)',
f'  - frequency 1 shift: {(CLKFREQ/d1):.3f} Mhz       (1 period = {d1} cycles @ {CLKFREQ} MHz clock)',
f'  - center frequency shift: {(fcenter/1000):.3f} MHz',
f'  - deviation from center : {fdeviation:.2f} kHz',
f'  - baud-rate {(b/1000):.2f} kBaud ({(CLKFREQ*1000000/b):.1f} instructions per symbol)',
f'  - occupied bandwith: {(b/1000 + 2*fdeviation):.2f} kHz']),end='\n\n')
if (fdeviation > 380.86):
    print('WARNING: the deviation is too large for the CC2500')
if (fdeviation > 1000):
    print('WARNING: the deviation is too large for the CC1352')
if (d0 < d1):
    print('WARNING: symbol 0 has been assigned to larger frequncy than symbol 1')
