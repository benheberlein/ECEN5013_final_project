/* @file main.c
 *  @brief This file contains the main routine.
 *
 *  This initializes the logger, command interface, 
 *  SDRAM, and starts the main command loop based on
 *  compile-time flags.
 *
 *  @author Ben Heberlein
 *  @bug No known bugs.
 */

/*************************************
 * Includes and definitions
 */

#include "stm32f4xx.h"
#include "err.h"
#include "log.h"
#include "cmd.h"
#include "sdram.h"
#ifdef __OV5642
#include "ov5642.h"
#endif
#ifdef __OV7670
#include "ov7670.h"
#endif
#include <stdint.h>

/**************************************
 * Private functions
 */

/**************************************
 * Public functions
 */

int main() {

    #ifdef __LOG  
    if (log_Init() == LOG_INFO_OK) {
        log_Log(LOG, LOG_INFO_OK, "Initialized logger.\0");
    } else {
        return -1;
    }
    #endif

    if (cmd_Init() == CMD_INFO_OK) {
        log_Log(CMD, CMD_INFO_OK, "Initialized command module.\0");
    } else {
        return -2;
    }

    #ifdef __STM32F429I_DISCOVERY
    sdram_status_t sd_st = sdram_Init();
    if (sd_st == SDRAM_INFO_OK) {
        log_Log(SDRAM, SDRAM_INFO_OK, "Initialized SDRAM interface.\0");
    } else {
        log_Log(SDRAM, sd_st, "Could not initialize SDRAM.\0");
    }
    #endif 

    #ifdef __OV5642
    ov5642_status_t ov_st = ov5642_Init();
    if (ov_st == OV5642_INFO_OK) {
        log_Log(OV5642, OV5642_INFO_OK, "Initialized 0V5642 camera module.\0");
    } else {
        log_Log(OV5642, ov_st, "Coud not initialize OV5642.\0");
    }

    ov_st = ov5642_Configure();

    ov_st = ov5642_Capture();

//    ov_st = ov5642_Transfer();
    #endif

    int i = 0;
    while(1) {
        *((uint8_t *)(0xd0100000) + i) = i;
        i++;
        if (i%100 == 0);
        i = 0;
    }

    #ifdef __OV7670
    ov7670_status_t ov_st = ov7670_Init();
  
    if (ov_st == OV7670_INFO_OK) {
        log_Log(OV5642, OV7670_INFO_OK, "Initialized 0V7670 camera module.\0");
    } else {
        log_Log(OV7670, ov_st, "Coud not initialize OV7670.\0");
    }

    ov_st = ov7670_Configure();

    ov_st = ov7670_Capture();

    ov_st = ov7670_Transfer();
    #endif

    // Start main loop
    cmd_status_t cmd_st = cmd_Loop();
    if (cmd_st != CMD_INFO_OK) {
        log_Log(CMD, cmd_st, "Exiting main.\0");
        
        return -3;
    }

    return 0;
}


