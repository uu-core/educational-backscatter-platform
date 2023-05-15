# Pico-Backscatter: carrier-receiver-backscatter (CC2500)
### Description
The implementation of a backscatter tag onboard control unit to generate an information-bearing 2-FSK signal using Raspberry Pi Pico PIO. The setup uses one Mikroe-1435 (CC2500) to generate a carrier and a second Mikroe-1435 (CC2500) to receive the backscattered signal.
The stdout (printf) has been directed to USB.

The example integrates the baseband, carrier generation and receiver. For further information on each component, please see the README files in:
- `baseband`
- `carrier-CC2500`
- `receiver-CC2500`

### Radio Settings
The radio settings and configuration can be generated using [SmartRF Studio](https://www.ti.com/tool/SMARTRFTM-STUDIO) and the datasheet of the corresponding module.
<br>Notice that the the configured baudrate of the Pico may be imprecise and differ from the one that the radio should be using. <br>To export the register settings compatible with the provided examples, you can add a new template with the following settings (Register Export -> New ->):
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
