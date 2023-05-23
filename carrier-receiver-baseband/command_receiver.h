
#ifndef COMMAND_RECEIVER_LIB
#define COMMAND_RECEIVER_LIB

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pico/util/queue.h"

extern uint32_t current_CENTER, current_DEVIATION, current_BAUDRATE, current_MIN_RX_BW, current_DIV0, current_DIV1, current_BAUD, current_DURATION;
extern mutex_t setting_mutex;

# define COMMAND_QUEUE_LENGTH 10
struct cmd_struct {
  char cmd;
  uint32_t  value1;
  uint32_t  value2;
  uint32_t  value3;
  uint32_t  value4;
};
typedef struct cmd_struct command_struct;

void printControlInfo();

void readInput_core1();

bool queued_command();

bool get_command(command_struct* ptr_cmd_event);

#endif