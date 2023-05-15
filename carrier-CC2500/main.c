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
 * 
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "carrier_CC2500.h"

#define CARRIER_FEQ     2450000000

void main() {
    stdio_init_all();
    spi_init(RADIO_SPI, 5 * 1000000); // SPI0 at 5MHz.
    gpio_set_function(RADIO_SCK, GPIO_FUNC_SPI);
    gpio_set_function(RADIO_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(RADIO_MISO, GPIO_FUNC_SPI);

    // Make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(RADIO_MOSI, RADIO_MISO, RADIO_SCK, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(CARRIER_CSN);
    gpio_set_dir(CARRIER_CSN, GPIO_OUT);
    gpio_put(CARRIER_CSN, 1);

    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(CARRIER_CSN, "SPI CS"));

    // Start carrier
    setupCarrier();
    set_frecuency_tx(CARRIER_FEQ);
    sleep_ms(1);
    printf("Started unmodulated carrier at 2450 MHz...\n");
    while (true) {
        sleep_ms(10);
    }
    stopCarrier(); // never reached
    printf("Stopped unmodulated carrier at 2450 MHz...\n");
}
