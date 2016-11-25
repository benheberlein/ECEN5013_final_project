/** @file main.c
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

    // Start main loop
    cmd_status_t cmd_st = cmd_Loop();
    if (cmd_st != CMD_INFO_OK) {
        log_Log(CMD, cmd_st, "Exiting main.\0");
        
        return -3;
    }

    return 0;
}


