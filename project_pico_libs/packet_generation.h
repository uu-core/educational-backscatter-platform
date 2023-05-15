/**
 * Tobias Mages & Wenqing Yan
 * 
 * support functions to generate payload data and function
 * 
 */

#ifndef PACKET_GERNATION_LIB
#define PACKET_GERNATION_LIB

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "packet_generation.h"

#define PAYLOADSIZE 14
#define HEADER_LEN  10 // 8 header + length + seq
#define buffer_size(x, y) (((x + y) % 4 == 0) ? ((x + y) / 4) : ((x + y) / 4 + 1)) // define the buffer size with ceil((PAYLOADSIZE+HEADER_LEN)/4)

#ifndef MINMAX
#define MINMAX
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

/*
 * obtain the packet header template for the corresponding radio
 */
uint8_t *packet_hdr_template(uint16_t receiver);

/* 
 * generate of a uniform random number.
 */
uint32_t rnd();

/* 
 * generate compressible payload sample
 * file_position provides the index of the next data byte (increments by 2 each time the function is called)
 */
extern uint16_t file_position;
uint16_t generate_sample();

/*
 * fill packet with 16-bit samples
 * include_index: shall the file index be included at the first two byte?
 * length: the length of the buffer which can be filled with data
*/
void generate_data(uint8_t *buffer, uint8_t length, bool include_index);


/* including a header to the packet:
 * - 8B header sequence
 * - 1B payload length
 * - 1B sequence number
 *
 * packet: buffer to be updated with the header
 * seq: sequence number of the packet
 */
void add_header(uint8_t *packet, uint8_t seq, uint8_t *header_template);

#endif