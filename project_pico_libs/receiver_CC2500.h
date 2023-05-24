/**
 * Tobias Mages & Wenqing Yan
 * 
 * Using the CC2500 to generate a 2450MHz unmodulated carrier with maximal output power of +1dBm.
 * 
 * GPIO 17 (pin 22) Chip select
 * GPIO 18 (pin 24) SCK/spi0_sclk
 * GPIO 19 (pin 25) MOSI/spi0_tx
 * GPIO 21 GDO0: interrupt for received sync word
 * 
 * The example uses SPI port 0. 
 * The stdout has been directed to USB.
 * To reconfigure the receiver see ../receiver-CC2500/README.md
 * 
 */

#ifndef RECEIVER_CC2500_LIB
#define RECEIVER_CC2500_LIB

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#define RADIO_SPI             spi0
#define RADIO_MISO              16
#define RADIO_MOSI              19
#define RADIO_SCK               18

#define RX_CSN                  17
#define RX_GDO0_PIN             21

#define RX_BUFFER_SIZE          64
#define EVENT_QUEUE_LENGTH      20 

#define SIDLE                 0x36
#define   SRX                 0x34
#define  SFRX                 0x3A
#define  SRES                 0x30

#define F_XOSC            26000000

#ifndef MINMAX
#define MINMAX
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef RF_SETTING
#define RF_SETTING
struct rf_setting {
  uint8_t address;
  uint8_t  value;
};
typedef struct rf_setting RF_setting;
#endif

struct packet_status {
  bool overflowed;
  uint8_t len;
  int32_t RSSI;
  bool CRCcheck;
  uint8_t LinkQualityIndicator;
};
typedef struct rf_setting RF_setting;
typedef struct rf_power RF_power;
typedef struct packet_status Packet_status;

/* Event queue */
typedef enum _event_t{
    no_evt          = 0,
    rx_assert_evt   = 1,
    rx_deassert_evt = 2
} event_t;

// Address Config = No address check 
// Base Frequency = 2456.596924 
// CRC Autoflush = false 
// CRC Enable = true 
// Carrier Frequency = 2456.596924 
// Channel Number = 0 
// Channel Spacing = 405.456543 
// Data Format = Normal mode 
// Data Rate = 98.587 
// Deviation = 355.468750 
// Device Address = 0 
// Manchester Enable = false 
// Modulated = true 
// Modulation Format = 2-FSK 
// Packet Length = 255 
// Packet Length Mode = Variable packet length mode. Packet length configured by the first byte after sync word 
// Preamble Count = 4 
// RX Filter BW = 812.500000 
// Sync Word Qualifier Mode = 30/32 sync word bits detected 
// TX Power = 0 
// Whitening = false 

extern RF_setting cc2500_receiver[20];

void cs_select_rx();

void cs_deselect_rx();

void write_strobe_rx(uint8_t cmd);

void write_register_rx(RF_setting set);

void write_registers_rx(RF_setting* sets, uint8_t len);

/* ISR */
void receiver_isr(uint gpio, uint32_t events);

void setupReceiver();

// continously listen for packets
void RX_start_listen();

// stop listening
void RX_stop_listen();

void print_registers_rx();

Packet_status readPacket(uint8_t *buffer);

void printPacket(uint8_t *packet, Packet_status status, uint64_t time_us);

event_t get_event(void);

//set datarate [baud]
uint32_t set_datarate_rx(uint32_t r_data);

//set filter bandwidth [Hz]
uint32_t set_filter_bandwidth_rx(uint32_t bw);

//set FSK frequency deviation [Hz]
uint32_t set_frequency_deviation_rx(uint32_t f_dev);

//set carrier frequency [Hz]
uint32_t set_frecuency_rx(uint32_t f_carrier);

#endif
