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

#ifndef CARRIER_CC2500_LIB
#define CARRIER_CC2500_LIB

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#define RADIO_SPI             spi0
#define RADIO_MISO              16
#define RADIO_MOSI              19
#define RADIO_SCK               18

#define CARRIER_CSN              5

#define SIDLE                 0x36
#define   STX                 0x35
#define  SRES                 0x30

#ifndef RF_SETTING
#define RF_SETTING
struct rf_setting {
  uint8_t address;
  uint8_t  value;
};
typedef struct rf_setting RF_setting;
#endif

struct rf_power {
  int8_t TX_power_dbm;
  uint8_t RegisterValue;
};
typedef struct rf_power RF_power;

// Address Config = No address check 
// Base Frequency = 2449.999756 
// Carrier Frequency = 2449.999756 
// Modulated = false 
extern RF_setting cc2500_unmodulated_2450MHz[16];

extern RF_power TX_power[18];

void cs_select_tx();

void cs_deselect_tx();

void write_strobe_tx(uint8_t cmd);

void write_register_tx(RF_setting set);

void write_registers_tx(RF_setting* sets, uint8_t len);

void setTXpower(RF_power setting);

void setupCarrier();

void startCarrier();

void stopCarrier();

//set carrier frequency [Hz]
void set_frecuency_tx(uint32_t f_carrier);

#endif
