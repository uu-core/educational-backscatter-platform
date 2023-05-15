# Pico-Backscatter: baseband
The implementation for backscatter tag onboard control unit to generate an information-bearing 2-FSK signal using Raspberry Pi Pico PIO.
The stdout (printf) has been directed to USB.

## Repo Organization
- `generate-backscatter-pio.py` provides a script to generate a PIO file for the desired shift frequencies and baud-rate.
- `backscatter.pio` provides an example output of `generate-backscatter-pio.py`
- `backscatter.c` contains an example of generating a pseudorandom payload and using the generated backscatter driver
- `CMakeList.txt`

## Frame Structure
| Header {10B} | Random Payload {Max. 60B} |
<br>**Header structure**
<br>| Preamble {4B} | SYNC words {4B} | Frame length {1B} | Sequence number {1B} |
<br> **RX: CC2500**
<br> | 0xaa 0xaa 0xaa 0xaa | 0xd3 0x91 0xd3 0x91 | 0x00 | 0x00 |
<br> **RX: CC1312**
<br> | 0xaa 0xaa 0xaa 0xaa | 0x93 0x0b 0x51 0xde | 0x00 | 0x00 |
<br> Please change the Macro variable RECEIVER, depending on your receiver setup.
<br>**Random Payload structure**
<br>| Pseudo sequence {2B} | random number {Max. 58B, which is equal to 29*(16-bit random number)}

## Usage of the PIO generation script

The PIO file is generated based on two clock-dividers from 125 MHz and a baud-rate. Each clock divider provides a frequency offset from the carrier for one symbol. The script prints the required radio settings in terms of the resulting center frequency offset and frequency deviation.
- Help: `python generate-backscatter-pio.py --help`
- Generating a PIO file: `python generate-backscatter-pio.py clock-divider-Symbol-0 clock-divider-Symbol-1 baud-rate output-file`

Example: `python generate-backscatter-pio.py 20 18 100000 ./backscatter.pio`
- 20     => 125MHz/20 = 6.25 MHz frequency offset from carrier for symbol 0
- 18     => 125MHz/18 = 6.94 MHz frequency offset from carrier for symbol 1
- 100000 => 100 kBaud
- notice that the output file has to match with the `CMakeLists.txt`

## Exercises questions
1. Why shall be used only _even_ clock dividers for generating the baseband?
2. How do the two frequency dividers and the baudrate affect the signal-to-noise ratio (SNR)? To maximize the SNR, how should the frequency deviation be? How should the baudrate be? How should the frequency offset be (remember its relation to $N_0$ due to the self-interference)?
3. What is the relation between the two clock dividers to achieve the best possible deviation for a given offset frequency? This relation leaves one degree of freedom. Generate a table which provides the achievable offset frequencies (center) together with the most desirable frequency deviation.
5. Question 2 highlighted a trade-off and question 3 highlighted a dependence. Create a simulation to identify the ideal trade-off parameter (table from question 3) for a given baudrate. How dependent are your results on the assumed distances and carrier?
Notice that you know the worst case baseband spectrum from the baudrate, can estimate the carrier spectrum using the measurements in `carrier-characteristics` and can take reasonable assumptions for the path-loss model and RX filter response.
6. Generate an experimental setup to estimate remaining parameters and calibrate the simulation.
7. Knowing the SNR at the receiver, the used signal bandwidth and assuming an additive white Gaussian noise (AWGN) channel: What is the *channel capacity* of your occupied resources based on the Shannon–Hartley theorem?
8. Assume you would like to optimize your parameters to maximize the channel capacity. Do you come to the same conclusion like for optimizing the SNR? Why/Why not?


### Antenna characteristics:
- the provided antennas are [Molex 2128601001](https://www.molex.com/molex/products/part-detail/antennas/2128601001)
- a quick guide of trace antennas can be found [here](https://www.ti.com/lit/an/swra351b/swra351b.pdf), which provides links to the most commen pcb antennas such as the [SWRA117 and its characteristics](https://www.ti.com/lit/an/swra117d/swra117d.pdf).

## Build the project
For **Linux and MacOS**:
1. set the sdk path {the path} with the absolute path\
```
 export PICO_SDK_PATH={the path}/pico-sdk
```
2. create build folder
```
mkdir build && cd build
```
3. prepare cmake build directory by running
```
cmake ..
```
4. build project
```
make
```

For **Windows**:
1. set the sdk path {the path} with the absolute path\
```
 setx PICO_SDK_PATH "{the path}/pico-sdk"
```
2. create build folder
```
mkdir build && cd build
```
3. prepare cmake build directory by running
```
cmake -G "NMake Makefiles" ..
nmake
```

## Reference
[Raspberry Pi Pico SDK Libraries and tools for C/C++ development on RP2040 microcontrollers](https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf).
<br>The details about PIO, please refer to [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf).
