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
#include <stdint.h>
#include "stm32f4xx_usart.h"

/**************************************
 * Private functions
 */

/**************************************
 * Public functions
 */

int main() {

    // Initalize logger and abort if there is an issue
    log_status_t ret = log_Init();
    if(ret != LOG_INFO_OK) {
        return -1;
    }

    // Initialize command interface
   
    while(1) {
        log_Log(LOG, LOG_ERR_UNKNOWN);
        log_Log(CMD, CMD_ERR_UNKNOWN, "bad\0");
    }

}


