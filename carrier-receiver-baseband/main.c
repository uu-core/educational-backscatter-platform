/**
 * Tobias Mages & Wenqing Yan
 * Backscatter PIO
 * 02-March-2023
 *
 * See the sub-projects ... for further information:
 *  - baseband
 *  - carrier-CC2500
 *  - receiver-CC2500
 *
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pico/multicore.h" 

#include "pico/util/queue.h"
#include "pico/binary_info.h"
#include "pico/util/datetime.h"
#include "hardware/spi.h"

#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "command_receiver.h"
#include "backscatter.h"
#include "carrier_CC2500.h"
#include "receiver_CC2500.h"
#include "packet_generation.h"


#define RADIO_SPI             spi0
#define RADIO_MISO              16
#define RADIO_MOSI              19
#define RADIO_SCK               18

#define TX_DURATION            250 // send a packet every 250ms (when changing baud-rate, ensure that the TX delay is larger than the transmission time)
#define RECEIVER              2500 // define the receiver board either 2500 or 1352
#define PIN_TX1                  6
#define PIN_TX2                 27
#define CLOCK_DIV0              20 // larger
#define CLOCK_DIV1              18 // smaller
#define DESIRED_BAUD         50000
#define TWOANTENNAS           true

#define CARRIER_FEQ     2450000000

/* Event queue for commands (start/stop uses zero values) */

/* just for the printout below. not actually used*/
#define PIO_BAUDRATE 100000
#define PIO_CENTER_OFFSET 6597222
#define PIO_DEVIATION 347222
#define PIO_MIN_RX_BW 794444

uint32_t current_CENTER, current_DEVIATION, current_BAUDRATE, current_MIN_RX_BW, current_DIV0, current_DIV1, current_BAUD, current_DURATION;
mutex_t setting_mutex;

void do_commands(){
    command_struct cmd_event;
    if(queued_command()){
        if (get_command(&cmd_event)){
            switch (cmd_event.cmd){
                case 'e':
                    printf("The input was invalid. Enter 'h' for further information on the interface.\n");
                    break;
                case 'h':
                    printControlInfo();
                    break;
                case 's':
                    RX_start_listen();
                    break;
                case 't':
                    RX_stop_listen();
                    break;
                case 'c':
                    RX_stop_listen();
                    setupReceiver();
                    uint32_t conf_CENTER, conf_DEVIATION, conf_BAUDRATE, conf_MIN_RX_BW;
                    conf_CENTER = set_frecuency_rx(cmd_event.value1);
                    conf_DEVIATION = set_frequency_deviation_rx(cmd_event.value2);
                    conf_BAUDRATE = set_datarate_rx(cmd_event.value3);
                    conf_MIN_RX_BW = set_filter_bandwidth_rx(cmd_event.value4);
                    mutex_enter_blocking(&setting_mutex);
                    current_CENTER=conf_CENTER;
                    current_DEVIATION=conf_DEVIATION;
                    current_BAUDRATE=conf_BAUDRATE;
                    current_MIN_RX_BW=conf_MIN_RX_BW;
                    mutex_exit(&setting_mutex);
                    printf("Used configuration: c %u ", conf_CENTER);
                    printf("%u ", conf_DEVIATION);
                    printf("%u ", conf_BAUDRATE);
                    printf("%u\n", conf_MIN_RX_BW);
                    RX_start_listen();
                    break;
                case 'b':
                    printf("Changing pio-state machine...\n");
                    mutex_enter_blocking(&setting_mutex);
                    current_DIV0 = cmd_event.value1;
                    current_DIV1 = cmd_event.value2;
                    current_BAUD = cmd_event.value3;
                    mutex_exit(&setting_mutex);
                    PIO pio = pio0;
                    uint sm = 0;
                    struct backscatter_config backscatter_conf;
                    uint16_t instructionBuffer[32] = {0}; // maximal instruction size: 32
                    if(backscatter_program_init(pio, sm, PIN_TX1, PIN_TX2, cmd_event.value1, cmd_event.value2, cmd_event.value3, &backscatter_conf, instructionBuffer, TWOANTENNAS)){
                        printf("Pio-state machine successfully changed.\n");
                    }else{
                        printf("Issue encountered. The state-machine has not been updated.\n");
                    }
                    break;
                default:
                    printf("Invalid command obtained.\n");
                    break;
            }
        }
    }
}

int main() {
    /* setup SPI */
    stdio_init_all();
    // Setup USB input on second core
    mutex_init(&setting_mutex);
    mutex_enter_blocking(&setting_mutex);
    current_CENTER = CARRIER_FEQ + PIO_CENTER_OFFSET;
    current_DEVIATION = PIO_DEVIATION;
    current_BAUDRATE = PIO_BAUDRATE;
    current_MIN_RX_BW = PIO_MIN_RX_BW;
    current_DIV0 = CLOCK_DIV0;
    current_DIV1 = CLOCK_DIV1;
    current_BAUD = PIO_BAUDRATE;
    current_DURATION = TX_DURATION;
    mutex_exit(&setting_mutex);
    multicore_reset_core1(); 
    multicore_launch_core1(readInput_core1); 

    /* SPI setup */
    spi_init(RADIO_SPI, 5 * 1000000); // SPI0 at 5MHz.
    gpio_set_function(RADIO_SCK, GPIO_FUNC_SPI);
    gpio_set_function(RADIO_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(RADIO_MISO, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(RADIO_MOSI, RADIO_MISO, RADIO_SCK, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(RX_CSN);
    gpio_set_dir(RX_CSN, GPIO_OUT);
    gpio_put(RX_CSN, 1);
    bi_decl(bi_1pin_with_name(RX_CSN, "SPI Receiver CS"));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(CARRIER_CSN);
    gpio_set_dir(CARRIER_CSN, GPIO_OUT);
    gpio_put(CARRIER_CSN, 1);
    bi_decl(bi_1pin_with_name(CARRIER_CSN, "SPI Carrier CS"));

    sleep_ms(5000);

    /* setup backscatter state machine */
    PIO pio = pio0;
    uint sm = 0;
    struct backscatter_config backscatter_conf;
    uint16_t instructionBuffer[32] = {0}; // maximal instruction size: 32
    backscatter_program_init(pio, sm, PIN_TX1, PIN_TX2, CLOCK_DIV0, CLOCK_DIV1, DESIRED_BAUD, &backscatter_conf, instructionBuffer, TWOANTENNAS);

    static uint8_t message[buffer_size(PAYLOADSIZE+2, HEADER_LEN)*4] = {0};  // include 10 header bytes
    static uint32_t buffer[buffer_size(PAYLOADSIZE, HEADER_LEN)] = {0}; // initialize the buffer
    static uint8_t seq = 0;
    uint8_t *header_tmplate = packet_hdr_template(RECEIVER);
    uint8_t tx_payload_buffer[PAYLOADSIZE];

    /* Setup carrier */
    setupCarrier();
    set_frecuency_tx(CARRIER_FEQ);
    sleep_ms(1);

    /* Start Receiver */
    event_t evt = no_evt;
    Packet_status status;
    uint8_t rx_buffer[RX_BUFFER_SIZE];
    uint64_t time_us;
    setupReceiver();
    set_frecuency_rx(CARRIER_FEQ + backscatter_conf.center_offset);
    set_frequency_deviation_rx(backscatter_conf.deviation);
    set_datarate_rx(backscatter_conf.baudrate);
    set_filter_bandwidth_rx(backscatter_conf.minRxBw);
    sleep_ms(1);
    RX_start_listen();
    printf("started listening\n");
    bool rx_ready = true;

    printControlInfo();

    /* loop */
    absolute_time_t next_release_1 = delayed_by_ms(get_absolute_time(), TX_DURATION);
    absolute_time_t next_release_2 = delayed_by_ms(next_release_1, TX_DURATION);
    while (true) {
        while(absolute_time_diff_us(get_absolute_time(), next_release_1));
        next_release_1 = next_release_2;
        next_release_2 = delayed_by_ms(next_release_1, TX_DURATION);

        do_commands();
        evt = get_event();
        switch(evt){
            case rx_assert_evt:
                // started receiving
                rx_ready = false;
            break;
            case rx_deassert_evt:
                // finished receiving
                time_us = to_us_since_boot(get_absolute_time());
                status = readPacket(rx_buffer);
                printPacket(rx_buffer,status,time_us);
                RX_start_listen();
                sleep_ms(1);
                rx_ready = true;
            //break;   // don't break still transmit next packet
            case no_evt:
                // backscatter new packet if receiver is listening
                if (rx_ready){
                    /* generate new data */
                    // generate_data(tx_payload_buffer, PAYLOADSIZE, true);
                    for(uint8_t i = 0; i < PAYLOADSIZE; i++){
                        tx_payload_buffer[i] = 0xA5;                                    // FOR DEMO: fixed payload of 0xA5
                    }

                    /* add header (10 byte) to packet */
                    add_header(&message[0], 0xA5, header_tmplate);                      // FOR DEMO: fixed sequence number of 0xA5
                    /* add payload to packet */
                    memcpy(&message[HEADER_LEN], tx_payload_buffer, PAYLOADSIZE);

                    /* casting for 32-bit fifo */
                    for (uint8_t i=0; i < buffer_size(PAYLOADSIZE, HEADER_LEN); i++) {
                        buffer[i] = ((uint32_t) message[4*i+3]) | (((uint32_t) message[4*i+2]) << 8) | (((uint32_t) message[4*i+1]) << 16) | (((uint32_t)message[4*i]) << 24);
                    }
                    /* put the data to FIFO (start backscattering) */
                    startCarrier();
                    sleep_ms(1); // wait for carrier to start
                    backscatter_send(pio,sm,buffer,buffer_size(PAYLOADSIZE, HEADER_LEN));
                    sleep_ms(ceil((((double) buffer_size(PAYLOADSIZE, HEADER_LEN))*8000.0)/((double) DESIRED_BAUD))+3); // wait transmission duration (+3ms)
                    stopCarrier();
                    /* increase seq number*/ 
                    seq++;
                }
            break;
        }
    }

    /* stop carrier and receiver - never reached */
    RX_stop_listen();
    stopCarrier();
}
