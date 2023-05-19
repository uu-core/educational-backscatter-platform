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
 *
 * The CC2500 can transmit and receive arbitrarily long packets. However, is FIFO is limited to 64 byte, 
 * out of which 4 byte are occupied by the length-field, sequence number and link quality information. 
 * This leaves 60 bytes for the payload.
 * To transmit larger payloads, it would be necessary to continoulsy empty the fifo while receiving a packet
 * which can lead to unwanted and timing dependent byte duplications as highlighted in the datasheet errata.
 * Therefore, we only start reading the fifo after the transmission is completed.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "pico/util/queue.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "receiver_CC2500.h"
#include "pico/multicore.h" 

# define COMMAND_QUEUE_LENGTH 10

#define CARRIER_FEQ     2450000000
/* 
 * The following macros are defined in the generated PIO header file 
 * We define them here manually here since this example does not require a PIO state machine.
*/
#define PIO_BAUDRATE 100000
#define PIO_CENTER_OFFSET 6597222
#define PIO_DEVIATION 347222
#define PIO_MIN_RX_BW 794444

/* Event queue for commands (start/stop uses zero values) */
struct cmd_struct {
  char cmd;
  uint32_t  value1;
  uint32_t  value2;
  uint32_t  value3;
  uint32_t  value4;
};
typedef struct cmd_struct command_struct;
queue_t command_queue;

void printControlInfo(){
    printf("The configuration can be changed using the following commands:\n   h (print this help message)\n   s (start receiving)\n   t (terminate/stop receiving)\n   c A B C D (configure A=center, B=deviation, C=baud, D=bandswidth all in Hz)\n");
    printf("The initial configuration is:\n  c 2456597222 ");  // somehow the macro sum doesn't print here
    printf("%u ", PIO_DEVIATION);
    printf("%u ", PIO_BAUDRATE);
    printf("%u\n\n", PIO_MIN_RX_BW);
}

static char command[100];
static int buff_pos = 0;  

void readInput_core1(){
    while(true){
        /* read input, parse input and put commands into the command queue */
        int input = getchar();
        if ((input == '\n' || input == '\r' || input == EOF) && buff_pos > 0) {
            command[buff_pos] = '\0'; 

            /* parse input and put to queue */
            command_struct cmd_event;
            char cmd;
            uint32_t  value1, value2, value3, value4;
            if(sscanf(command, "%c %u %u %u %u", &cmd, &value1, &value2, &value3, &value4) != 5){
                if(sscanf(command, "%c", &cmd) != 1){
                    cmd_event.cmd = 'e'; // e for invalid input (error)
                    cmd_event.value1 = 0;
                    cmd_event.value2 = 0;
                    cmd_event.value3 = 0;
                    cmd_event.value4 = 0;
                    queue_try_add(&command_queue, &cmd_event);
                }else{
                    switch (cmd){
                        case 'h':
                            cmd_event.cmd = 'h';
                            cmd_event.value1 = 0;
                            cmd_event.value2 = 0;
                            cmd_event.value3 = 0;
                            cmd_event.value4 = 0;
                            queue_try_add(&command_queue, &cmd_event);
                            break;
                        case 's':
                            cmd_event.cmd = 's';
                            cmd_event.value1 = 0;
                            cmd_event.value2 = 0;
                            cmd_event.value3 = 0;
                            cmd_event.value4 = 0;
                            queue_try_add(&command_queue, &cmd_event);
                            break;
                        case 't':
                            cmd_event.cmd = 't';
                            cmd_event.value1 = 0;
                            cmd_event.value2 = 0;
                            cmd_event.value3 = 0;
                            cmd_event.value4 = 0;
                            queue_try_add(&command_queue, &cmd_event);
                            break;
                        default:
                            cmd_event.cmd = 'e'; // e for invalid input (error)
                            cmd_event.value1 = 0;
                            cmd_event.value2 = 0;
                            cmd_event.value3 = 0;
                            cmd_event.value4 = 0;
                            queue_try_add(&command_queue, &cmd_event);
                            break;
                    }
                }
            }else{
                switch (cmd){
                    case 'c':
                        cmd_event.cmd = 'c';
                        cmd_event.value1 = value1;
                        cmd_event.value2 = value2;
                        cmd_event.value3 = value3;
                        cmd_event.value4 = value4;
                        queue_try_add(&command_queue, &cmd_event);
                        break;
                    default:
                        cmd_event.cmd = 'e'; // e for invalid input (error)
                        cmd_event.value1 = 0;
                        cmd_event.value2 = 0;
                        cmd_event.value3 = 0;
                        cmd_event.value4 = 0;
                        queue_try_add(&command_queue, &cmd_event);
                        break;
                }
            }
            buff_pos = 0; 
        } else {
            command[buff_pos] = (char) input; 
            buff_pos++; 
        }
    }
}

void do_commands(){
    if(!queue_is_empty(&command_queue)){
        command_struct cmd_event;
        if (queue_try_remove(&command_queue,&cmd_event)){
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
                    set_frecuency_rx(cmd_event.value1);
                    set_frequency_deviation_rx(cmd_event.value2);
                    set_datarate_rx(cmd_event.value3);
                    set_filter_bandwidth_rx(cmd_event.value4);
                    RX_start_listen();
                    break;
                default:
                    printf("Invalid command obtained.\n");
                    break;
            }
        }
    }
}

void main() {
    // stdio init
    stdio_init_all();
    // Setup USB input on second core
    queue_init(&command_queue, sizeof(command_struct), COMMAND_QUEUE_LENGTH); /* command queue setup */
    while(queue_try_remove(&command_queue, NULL));                            /* Reset the queue     */
    multicore_reset_core1(); 
    multicore_launch_core1(readInput_core1); 

    // setup SPI
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

    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(RX_CSN, "SPI CS"));

    // Start receiver
    sleep_ms(5000);
    event_t evt = no_evt;
    Packet_status status;
    uint8_t buffer[RX_BUFFER_SIZE];
    setupReceiver();
    set_frecuency_rx(CARRIER_FEQ + PIO_CENTER_OFFSET);
    set_frequency_deviation_rx(PIO_DEVIATION);
    set_datarate_rx(PIO_BAUDRATE);
    set_filter_bandwidth_rx(PIO_MIN_RX_BW);
    sleep_ms(1);
    RX_start_listen();
    printControlInfo();

    while (true) {
        do_commands();
        evt = get_event();
        switch(evt){
            case rx_assert_evt:
                // started receiving
            break;
            case rx_deassert_evt:
                // finished receiving
                uint64_t time_us = to_us_since_boot(get_absolute_time());
                status = readPacket(buffer);
                printPacket(buffer,status,time_us);
                RX_start_listen();
            break;
            case no_evt:
            break;
        }
        sleep_us(10);
    }
    RX_stop_listen(); // never reached
}
