/** @file sdram.h
 *  @brief Function prototypes for the SDRAM functions
 *
 *  This contains the prototypes for the SDRAM functions
 *  based on compiler directives.
 *
 *  If __STM32F429I_DISCOVERY is defined, we use the 
 *  implementation of the SDRAM interface from the
 *  discovery board libraries.
 *
 *  @author Ben Heberlein
 *  @bug No known bugs.
 */

#ifndef __SDRAM_H
#define __SDRAM_H

/*************************************
 * Includes and definitions
 */

#include <stdint.h>
#include "err.h"

/**************************************
 * Private functions
 */

/**************************************
 * Public functions
 */

/** @brief Initialize the SDRAM interface
 *
 *  This will initialize the SDRAM interface either
 *  using the discovery board library or a custom
 *  implementations         
 *
 *  @return a status code of the type sdram_status_t
 */
sdram_status_t sdram_Init();

# endif /* __TEMPLATE_H */
