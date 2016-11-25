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
    // Initalize logger and abort if there is an issue
    if (log_Init() != LOG_INFO_OK) {
        return -1;
    }
    log_Log(LOG, LOG_INFO_OK, "Initialized logger.\0");
#endif

    // Initialize cmd module and abort if there's an issue
    if (cmd_Init() != CMD_INFO_OK) {
        return -2;
    }
    log_Log(CMD, CMD_INFO_OK, "Initiialized command module.\0");


    while(1) {
    }

}


