/*
 * Tobias Mages & Wenqing Yan
 * Course: Wireless Communication and Networked Embedded Systems, Project VT2023
 * Backscatter PIO
 * 29-March-2023
 */

#include "backscatter.h"

// repeat the instruction until the desired delay has past
int16_t repeat(uint16_t* instructionBuffer, int16_t delay, uint32_t asm_instr, uint8_t *length, uint16_t max_delay){
    while(delay > 0){
        uint8_t delay_part = min(max_delay, delay) - 1;
        instructionBuffer[(*length)] = asm_instr | (((max_delay-1) & delay_part) << 8);
        delay = delay - (delay_part + 1);
        (*length)++;
    }
}

// how many instructions are needed to create this delay?
uint8_t instructionCount(uint16_t delay, uint16_t max_delay){
    if (delay % max_delay == 0){
        return delay/max_delay;
    }else{
        return delay/max_delay + 1;
    }
}

bool generatePIOprogram(uint16_t d0,uint16_t d1, uint32_t baud, uint16_t* instructionBuffer, struct pio_program *backscatter_program, bool twoAntennas){
    // compute label positions
    uint16_t MAX_ASMDELAY = 0x0020; // 32
    uint16_t OPT_SIDE_1   = 0x0000;
    uint16_t OPT_SIDE_0   = 0x0000;
    if (twoAntennas){
        MAX_ASMDELAY = 0x0008;     //   8 
        OPT_SIDE_1   = 0x1800;
        OPT_SIDE_0   = 0x1000;
    }
    uint8_t get_symbol_label = 3;
    uint8_t send_1_label = 5;
    uint8_t loop_1_label = send_1_label + 1;
    int16_t lastPeriodCycles1 = (((uint32_t) CLKFREQ*1000000)/baud - 4) % ((uint32_t) d1);
    int16_t lastPeriodCycles0 = (((uint32_t) CLKFREQ*1000000)/baud - 4) % ((uint32_t) d0);
    int16_t tmp1 = min(lastPeriodCycles1, d1/2);
    int16_t tmp0 = min(lastPeriodCycles0, d0/2);
    /*                                           pull high                 pull low            jmp                 high                                      low                            jmp  */
    uint8_t send_0_label = loop_1_label + instructionCount(d1/2, MAX_ASMDELAY) + instructionCount(d1/2 - 1, MAX_ASMDELAY) + 1 + instructionCount(tmp1, MAX_ASMDELAY) + instructionCount(max(0,lastPeriodCycles1-tmp1), MAX_ASMDELAY) + 1;
    uint8_t loop_0_label = send_0_label + 1;

    // check that the program will fit into memory
    /*                      pull high                pull low            jmp                        high                              low                               jmp  */
    if(loop_0_label + instructionCount(d0/2, MAX_ASMDELAY) + instructionCount(d0/2 - 1, MAX_ASMDELAY) + 1 + instructionCount(tmp0, MAX_ASMDELAY) + instructionCount(max(0,lastPeriodCycles0-tmp0), MAX_ASMDELAY) + 1 >= 32){
        printf("ERROR: The clock dividers are too small. The program would not fit into the state-machine instruction memory. Alternatively, you can disable the second antenna. This increaes the maximal delay per instruction from 8 to 32 cycles and thus significanlty reduces the required code space.");
        return false;
    }

    // generate state machine
    instructionBuffer[0] = ASM_SET_PINS | OPT_SIDE_1 | 1;           //  0: set    pins, 1         side 1
    instructionBuffer[1] = ASM_OUT | (ASM_ISR_REG << 5);            //  1: out    isr, 32   (NOTE: 32=0)
    instructionBuffer[2] = ASM_OUT | (ASM_Y_REG   << 5);            //  2: out    y, 32     (NOTE: 32=0)
    instructionBuffer[3] = ASM_OUT | (ASM_X_REG   << 5) |  1;       //  3: out    x, 1   
    instructionBuffer[4] = ASM_JMP_NOTX | (0x1F & send_0_label);    //  4: jmp    !x, send_0_label
    /*       symbol 1      */
    instructionBuffer[5] = ASM_MOV | (ASM_X_REG << 5) | ASM_Y_REG;  //  5: mov    x, y                  
    uint8_t length = 6;
    // full periods
    repeat(instructionBuffer, d1/2,     ASM_SET_PINS | OPT_SIDE_1 | 1, &length, MAX_ASMDELAY);   //    6: set    pins, 1         side 1 [delay] 
    repeat(instructionBuffer, d1/2 - 1, ASM_SET_PINS | OPT_SIDE_0 | 0, &length, MAX_ASMDELAY);   //  ...: set    pins, 0         side 0 [delay] 
    instructionBuffer[length] = ASM_JMP_XMM | (0x1F & loop_1_label);                             //  ...: jmp    x--, loop_1_label
    length++;
    // remaining period to fill symbol time
    repeat(instructionBuffer,                          tmp1, ASM_SET_PINS | OPT_SIDE_1 | 1, &length, MAX_ASMDELAY); //  ...: set    pins, 1         side 1 [delay] 
    repeat(instructionBuffer, max(0,lastPeriodCycles1-tmp1), ASM_SET_PINS | OPT_SIDE_0 | 0, &length, MAX_ASMDELAY); //  ...: set    pins, 0         side 0 [delay] 
    instructionBuffer[length] = ASM_JMP | get_symbol_label;               // ...: jmp    get_symbol_label
    length++;
    /*       symbol 0       */
    instructionBuffer[length] = ASM_MOV | (ASM_X_REG << 5) | ASM_ISR_REG, // ...: mov    x, isr  
    length++; 
    // full periods
    repeat(instructionBuffer, d0/2,     ASM_SET_PINS | OPT_SIDE_1 | 1, &length, MAX_ASMDELAY);    // ...: set    pins, 1         side 1 [delay_part] 
    repeat(instructionBuffer, d0/2 - 1, ASM_SET_PINS | OPT_SIDE_0 | 0, &length, MAX_ASMDELAY);    // ...: set    pins, 0         side 0 [delay_part] 
    instructionBuffer[length] = ASM_JMP_XMM | (0x1F & loop_0_label);      //  ...: jmp    x--, loop_0_label
    length++;
    // remaining period to fill symbol time
    repeat(instructionBuffer,                          tmp0, ASM_SET_PINS | OPT_SIDE_1 | 1, &length, MAX_ASMDELAY);  //  ...: set    pins, 1         side 1 [delay_part] 
    repeat(instructionBuffer, max(0,lastPeriodCycles0-tmp0), ASM_SET_PINS | OPT_SIDE_0 | 0, &length, MAX_ASMDELAY);  //  ...: set    pins, 0         side 0 [delay_part] 
    instructionBuffer[length] = ASM_JMP | get_symbol_label; // ...: jmp    get_symbol_label

    // configure program origin and length
    backscatter_program->instructions = instructionBuffer;
    backscatter_program->length = length+1;
    backscatter_program->origin = -1;
    return true;
}

/* 
    - based on d0/d1/baud, the modulation parameters will be computed and returned in the struct backscatter_config 
    - pin2 is ignored if twoAntennas==false
*/
void backscatter_program_init(PIO pio, uint sm, uint pin1, uint pin2, uint16_t d0, uint16_t d1, uint32_t baud, struct backscatter_config *config, uint16_t *instructionBuffer, bool twoAntennas){
    pio_sm_set_enabled(pio, sm, false); // stop state machine if running
    pio_clear_instruction_memory(pio);
    // print warning at invalid settings
    if(d0 % 2 != 0){
        printf("WARNING: the clock divider d0 has to be an even integer. The state-machine may not function correctly");
    }
    if(d1 % 2 != 0){
        printf("WARNING: the clock divider d1 has to be an even integer. The state-machine may not function correctly");
    }
    // correct baud-rate
    if(((uint32_t) (CLKFREQ*pow(10,6))) % baud != 0){
        uint32_t baud_new = round(((uint32_t) (CLKFREQ*pow(10,6))) / round(((double) CLKFREQ*pow(10,6)) / ((double) baud)));
        printf("WARNING: a baudrate of %d Baud is not achievable with a %d MHz clock.\nTherefore, the closest achievable baud-rate %d Baud will be used.\n", baud, CLKFREQ, baud_new);
        baud = baud_new;
    }
    // generate pio-program
    struct pio_program backscatter_program;
    generatePIOprogram(d0,d1,baud, instructionBuffer, &backscatter_program, twoAntennas);
    uint offset = 0;
    pio_add_program_at_offset(pio, &backscatter_program, offset); // load program
    /* print state-machine instructions */
    //printf("state-machine length: %d\n", backscatter_program.length);
    //for (uint16_t t = 0; t < backscatter_program.length; t++){
    //    printf("0x%04x\n",backscatter_program.instructions[t]);
    //}
    // configure the state-machine
    pio_gpio_init(pio, pin1);
    pio_sm_set_consecutive_pindirs(pio, sm, pin1, 1, true);
    if(twoAntennas){
        pio_gpio_init(pio, pin2);
        pio_sm_set_consecutive_pindirs(pio, sm, pin2, 1, true);    
    }
    // setup default state-machine config
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset, offset + backscatter_program.length-1); 
    // setup specific state-machine config
    sm_config_set_set_pins(&c, pin1, 1);
    if(twoAntennas){
        sm_config_set_sideset(&c, 2, true, false);
        sm_config_set_sideset_pins(&c, pin2);
    }
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX); // We only need TX, so get an 8-deep FIFO (join RX and TX FIFO)
    sm_config_set_out_shift(&c, false, true, 32);  // OUT shifts to left (MSB first), autopull after every 32 bit
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
    uint32_t reps0 = ((CLKFREQ*1000000/baud - 4) / d0) - 1;
    uint32_t reps1 = ((CLKFREQ*1000000/baud - 4) / d1) - 1;
    pio_sm_put_blocking(pio, sm, reps0); // -1 is requried since JMP 0-- is still true
    pio_sm_put_blocking(pio, sm, reps1); // -1 is required since JMP 0-- is still true

    // compute configuration parameters
    uint32_t fcenter    = (CLKFREQ*1000000/d0 + CLKFREQ*1000000/d1)/2;
    uint32_t fdeviation = abs(round((((double) CLKFREQ*1000000)/((double) d1)) - ((double) fcenter)));
    config->baudrate    = baud;
    config->center_offset = round(fcenter);
    config->deviation   = round(fdeviation);
    config->minRxBw     = round((baud + 2*fdeviation));
    
    if (fdeviation > 380000){
        printf("WARNING: the deviation is too large for the CC2500\n");
    }
    if (fdeviation > 1000000){
        printf("WARNING: the deviation is too large for the CC1352\n");
    }
    if (d0 < d1){
        printf("WARNING: symbol 0 has been assigned to larger frequncy than symbol 1\n");
    }

    printf("Computed baseband settings: \n- baudrate: %d\n- Center offset: %d\n- deviation: %d\n- RX Bandwidth: %d\n", config->baudrate, config->center_offset, config->deviation, config->minRxBw);
}

void backscatter_send(PIO pio, uint sm, uint32_t *message, uint32_t len) {
    for(uint32_t i = 0; i < len; i++){
        pio_sm_put_blocking(pio, sm, message[i]); // set pin back to low
    }
    sleep_ms(1); // wait for transmission to finish
}
