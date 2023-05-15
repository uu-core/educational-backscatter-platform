/**
 * Tobias Mages & Wenqing Yan
 *
 * Using the CC2500 to generate a 2450MHz unmodulated carrier with maximal output power of +1dBm.
 *
 * GPIO  5 Chip select
 * GPIO 18 SCK/spi0_sclk
 * GPIO 19 MOSI/spi0_tx
 *
 * The example uses SPI port 0.
 * The stdout has been directed to USB.
 * To reconfigure the carrier frequency see ../carrier-CC2500/README.md
 *
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "receiver_CC2500.h"
#include "carrier_CC2500.h"

// Address Config = No address check
// Base Frequency = 2449.999756
// Carrier Frequency = 2449.999756
// Modulated = false
RF_setting cc2500_unmodulated_2450MHz[16] = {
  {.address = 0x00, .value = 0x0B}, // CC2500_IOCFG2: GDO2Output Pin Configuration
  {.address = 0x02, .value = 0x0C}, // CC2500_IOCFG0: GDO0Output Pin Configuration
  {.address = 0x08, .value = 0x12}, // CC2500_PKTCTRL0: Packet Automation Control
  {.address = 0x0b, .value = 0x0B}, // CC2500_FSCTRL1: Frequency Synthesizer Control
  {.address = 0x0e, .value = 0x3B}, // CC2500_FREQ1: Frequency Control Word, Middle Byte
  {.address = 0x0f, .value = 0x13}, // CC2500_FREQ0: Frequency Control Word, Low Byte
  {.address = 0x10, .value = 0x78}, // CC2500_MDMCFG4: Modem Configuration
  {.address = 0x11, .value = 0x93}, // CC2500_MDMCFG3: Modem Configuration
  {.address = 0x12, .value = 0xB0}, // CC2500_MDMCFG2: Modem Configuration
  {.address = 0x15, .value = 0x44}, // CC2500_DEVIATN: Modem Deviation Setting
  {.address = 0x18, .value = 0x18}, // CC2500_MCSM0: Main Radio Control State Machine Configuration
  {.address = 0x19, .value = 0x16}, // CC2500_FOCCFG: Frequency Offset Compensation Configuration
  {.address = 0x1b, .value = 0x43}, // CC2500_AGCCTRL2: AGC Control
  {.address = 0x22, .value = 0x11}, // CC2500_FREND0: Front End TX configuration
  {.address = 0x25, .value = 0x00}, // CC2500_FSCAL1: Frequency Synthesizer Calibration
  {.address = 0x26, .value = 0x11}, // CC2500_FSCAL0: Frequency Synthesizer Calibration
};

RF_power TX_power[] = {
    {.TX_power_dbm = -55, .RegisterValue = 0x00}, //  0
    {.TX_power_dbm = -30, .RegisterValue = 0x50}, //  1
    {.TX_power_dbm = -28, .RegisterValue = 0x44}, //  2
    {.TX_power_dbm = -26, .RegisterValue = 0xC0}, //  3
    {.TX_power_dbm = -24, .RegisterValue = 0x84}, //  4
    {.TX_power_dbm = -22, .RegisterValue = 0x81}, //  5
    {.TX_power_dbm = -20, .RegisterValue = 0x46}, //  6
    {.TX_power_dbm = -18, .RegisterValue = 0x93}, //  7
    {.TX_power_dbm = -16, .RegisterValue = 0x55}, //  8
    {.TX_power_dbm = -14, .RegisterValue = 0x8D}, //  9
    {.TX_power_dbm = -12, .RegisterValue = 0xC6}, // 10
    {.TX_power_dbm = -10, .RegisterValue = 0x97}, // 11
    {.TX_power_dbm =  -8, .RegisterValue = 0x6E}, // 12
    {.TX_power_dbm =  -6, .RegisterValue = 0x7F}, // 13
    {.TX_power_dbm =  -4, .RegisterValue = 0xA9}, // 14
    {.TX_power_dbm =  -2, .RegisterValue = 0xBB}, // 15
    {.TX_power_dbm =   0, .RegisterValue = 0xFE}, // 16
    {.TX_power_dbm =  +1, .RegisterValue = 0xFF}, // 17
};

void cs_select_tx() {
    asm volatile("nop \n nop \n nop");
    gpio_put(CARRIER_CSN, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

void cs_deselect_tx() {
    asm volatile("nop \n nop \n nop");
    gpio_put(CARRIER_CSN, 1);
    asm volatile("nop \n nop \n nop");
}

void write_strobe_tx(uint8_t cmd) {
    cs_select_tx();
    spi_write_blocking(RADIO_SPI, &cmd, 1);
    cs_deselect_tx();
    sleep_ms(1);
}

void write_register_tx(RF_setting set) {
    uint8_t buf[2];
    buf[0] = set.address;
    buf[1] = set.value;
    cs_select_tx();
    spi_write_blocking(RADIO_SPI, buf, 2);
    cs_deselect_tx();
    sleep_ms(1);
}

void write_registers_tx(RF_setting* sets, uint8_t len) {
    uint8_t buf[2];
    cs_select_tx();
    for (int i = 0; i < len; i++) {
        buf[0] = sets[i].address;
        buf[1] = sets[i].value;
        spi_write_blocking(RADIO_SPI, buf, 2);
    }
    cs_deselect_tx();
}

RF_setting read_register_tx(uint8_t address) {
    uint8_t buf[2] = {0, 0};
    cs_select_tx();
    spi_read_blocking(RADIO_SPI, address+0x80, buf, 2);
    cs_deselect_tx();
    sleep_ms(1);
    return (RF_setting){.address = address, .value = buf[1]};
}

void setTXpower(RF_power setting) {
    uint8_t buf[3];
    cs_select_tx();
    buf[0] = (1 << 6) | 0x3E; // enable burst write to 0x3E
    buf[1] = setting.RegisterValue;
    buf[2] = setting.RegisterValue;
    spi_write_blocking(RADIO_SPI, buf, 3);
    cs_deselect_tx();
}


void setupCarrier(){
    write_strobe_tx(SRES);  // in case of reset without power loss - reset manually
    sleep_us(100);
    write_strobe_tx(SIDLE); // ensure IDLE mode with command strobe: SIDLE
    write_registers_tx(cc2500_unmodulated_2450MHz,16);
    setTXpower(TX_power[17]); // set +1dBm output power (max)
}

void startCarrier(){
    write_strobe_tx(STX); // start carrier (enter TX mode with command strobe: STX)
}

void stopCarrier(){
    write_strobe_tx(SIDLE); // stop carrier (enter IDLE mode with command strobe: SIDLE)
}

void set_frecuency_tx(uint32_t f_carrier)
{
// Test read_register_tx
//    RF_setting a = {.address = 0x13, .value = 0xab};
//    write_register_tx(a);
//    RF_setting b = read_register_tx(0x13);
//    printf("debug return %02x\n", b.value);
    
    write_strobe_tx(SIDLE); // ensure IDLE mode with command strobe: SIDLE
    
    // see datasheet, section 21
    // approach: chose start frequency as close as possible to f_carrier, correct with channel
    uint32_t freq = floor(f_carrier *((double) (1 << 16)) / ((double) F_XOSC));
    uint8_t channel = 0;
    uint8_t channspc_e = 0;
    uint8_t channspc_m = floor(((((double) f_carrier) * (1 << 16)) / ((double) F_XOSC) - freq - (1 << 6)) * (1 << 2));

    // print new value
    uint32_t f_carrier_calculated = floor(((double) F_XOSC) * (freq + (double) channel*(256+channspc_m)/((double) (1 << 2))) / ((double) (1 << 16)));
    printf("set tx f_carrier [%u %u %u %u] %u\n", freq, channel, channspc_e, channspc_m, f_carrier_calculated);
    
    // CHANNR, FREQ2, FREQ1, FREQ0, MDMCFG1, MDMCFG1
    RF_setting mdmcfg1 = read_register_tx(0x13);
    RF_setting set[6] = {
        {.address = 0x0a, .value = channel},
        {.address = 0x0d, .value = ((freq & 0x007f0000) >> 16)},
        {.address = 0x0e, .value = ((freq & 0x0000ff00) >> 8)},
        {.address = 0x0f, .value = (freq & 0x000000ff)},
        {.address = 0x13, .value = (mdmcfg1.value & 0xf0) + (channspc_e & 0x03)},
        {.address = 0x14, .value = channspc_m}
    };
    //printf("debug %02x %02x %02x %02x %02x %02x\n", set[0].value, set[1].value, set[2].value, set[3].value, set[4].value, set[5].value);
    write_registers_tx(set,6);
}
