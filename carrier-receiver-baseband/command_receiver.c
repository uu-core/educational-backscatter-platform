
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pico/util/queue.h"
#include "command_receiver.h"

void printControlInfo(){
    mutex_enter_blocking(&setting_mutex);
    uint32_t _current_CENTER = current_CENTER;
    uint32_t _current_DEVIATION = current_DEVIATION;
    uint32_t _current_BAUDRATE = current_BAUDRATE;
    uint32_t _current_MIN_RX_BW = current_MIN_RX_BW;
    uint32_t _current_DIV0 = current_DIV0;
    uint32_t _current_DIV1 = current_DIV1;
    uint32_t _current_BAUD = current_BAUD;
    uint32_t _current_DURATION = current_DURATION;
    mutex_exit(&setting_mutex);
    printf("The configuration can be changed using the following commands:\n   h (print this help message)\n   s (start receiving)\n   t (terminate/stop receiving)\n   c A B C D (configure receiver A=center, B=deviation, C=baud, D=bandswidth all in Hz)\n   b A B C (configure backscatter A=divider1, B=divider2, C=baud)\n\n");
    printf("The current receiver configuration is:\n  c %u ", _current_CENTER);
    printf("%u ", _current_DEVIATION);
    printf("%u ", _current_BAUDRATE);
    printf("%u\n\n", _current_MIN_RX_BW);
    printf("The current backscatter configuration is:\n  b %u ", _current_DIV0);
    printf("%u ", _current_DIV1);
    printf("%u\n\n", _current_BAUD);
    printf("The backscattering runs continously every %u ms.\n\n", _current_DURATION);
}

queue_t command_queue;
static char command[100];
static int buff_pos = 0;  

void readInput_core1(){
    
    queue_init(&command_queue, sizeof(command_struct), COMMAND_QUEUE_LENGTH); /* command queue setup */
    while(queue_try_remove(&command_queue, NULL));                            /* Reset the queue     */

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
                if(sscanf(command, "%c %u %u %u", &cmd, &value1, &value2, &value3) != 4){
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
                        case 'b':
                            cmd_event.cmd = 'b';
                            cmd_event.value1 = value1;
                            cmd_event.value2 = value2;
                            cmd_event.value3 = value3;
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

bool queued_command(){
    return !queue_is_empty(&command_queue);
}

bool get_command(command_struct* ptr_cmd_event){
    return queue_try_remove(&command_queue, ptr_cmd_event);
}