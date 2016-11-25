/** @file main.c
 *  @brief This file contains the main
 *  routine.
 *
 *  This initializes various modules
 *  and controls functionality.
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
#include <stdint.h>
#include "stm32f4xx_usart.h"

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

    // Start main loop
    cmd_status_t st = cmd_Loop();
    if (st != CMD_INFO_OK) {
        log_Log(CMD, st, "Exiting main.\0");
        return -3;
    }

    return 0;
}


