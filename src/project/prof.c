/** @file prof.c
 *  @brief Implemenation of the profiler functions.
 *
 *  This contains the implementations of the profiler 
 *  functions. The main profiler function is defined as a 
 *  preprocessor macro in the prof.h file.
 *
 *  @author Ben Heberlein
 *  @bug No known bugs.
 */

/*************************************
 * Includes and definitions
 */

#include "prof.h"
#include "err.h"
#include <stdint.h>

/**************************************
 * Private functions
 */

#ifdef __PROF
prof_status_t prof_itoa(uint8_t *str, uint16_t data) {
    return PROF_INFO_OK;
}

prof_status_t prof_start() {
    return PROF_INFO_OK;
}

prof_status_t prof_stop() {
    return PROF_INFO_OK;
}

/**************************************
 * Public functions
 */

#ifdef __PROF
prof_status_t prof_Init() {

}

/* Preprocessor macro prof_Profile(x) defined in prof.h
 * This will be the main profiler function called by
 * the user.
 */
#endif

