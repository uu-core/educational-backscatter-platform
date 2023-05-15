# Pico-Backscatter: receiver-CC2500
### Decription
Receiver example using the Mikroe-1435 (CC2500) on the Pico.
The stdout (printf) has been directed to USB.

## Backscatter Tag Configuration
Please change the Macro variable RECEIVER to 2500.

### Radio wiring information

The example here uses SPI 0.

   * GPIO 17 (pin 22) Chip select
   * GPIO 18 (pin 24) SCK/spi0_sclk
   * GPIO 19 (pin 25) MOSI/spi0_tx
   * GPIO 21 GDO0: interrupt for received sync word


### Receiver configuration

The CC2500 can transmit and receive arbitrarily long packets. However, is FIFO is limited to 64 byte, out of which 4 byte are occupied by the length-field, sequence number and link quality information. This leaves 60 bytes for the payload.

To transmit larger payloads, it would be necessary to continoulsy empty the fifo while receiving a packet which can lead to unwanted and timing dependent byte duplications as highlighted in the [datasheet errata](https://www.ti.com/lit/er/swrz002e/swrz002e.pdf).

### Radio Settings
The radio settings and configuration can be generated using [SmartRF Studio](https://www.ti.com/tool/SMARTRFTM-STUDIO) and the datasheet of the corresponding module. Notice that the the configured baudrate of the Pico may be imprecise and differ from the one that the radio should be using. To export the register settings compatible with the provided examples, you can add a new template with the following settings (Register Export -> New ->):
- Header
    ```
    #ifndef RF_SETTING
    #define RF_SETTING
    struct rf_setting {
        uint8_t address;
        uint8_t  value;
    };
    typedef struct rf_setting RF_setting;
    #endif

    RF_setting radio_settings[] = {
    ```
- Body:
    ```
        {.address = 0x@ah@,@<<@ .value = 0x@VH@}, // @CHIPID@_@RN@: @Rd@
    ```
- Footer `};`

Additionally, notice that the exported register configuration of SmartRF Studio does not contain the transmission power setting, which is configured in the PA-Table.


### Build the project
Please follow the installation guidance in [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).

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
mkdir build
cd build
```
3. prepare cmake build directory by running
```
cmake -G "NMake Makefiles" ..
nmake
```

## Reference
[Raspberry Pi Pico SDK Libraries and tools for C/C++ development on RP2040 microcontrollers](https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf).
<br>The details about PIO, please refer to [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf).
