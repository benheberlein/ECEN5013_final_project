/** @file prof.h
 *  @brief Function prototypes for the profiler.
 *
 *  This contains the prototypes and macros for the
 *  profiler module.
 *   
 *  @author Ben Heberlein
 *  @bug No known bugs.
 */

#ifndef __PROF_H
#define __PROF_H

/*************************************
 * @name Includes and definitions
 */

#include "err.h"
#include <stdint.h>

/**************************************
 * @name Private functions
 */

#ifdef __PROF
/** @brief Convert an integer to an ASCII representation
 *
 *  Converts a positive integer to its ASCII string
 *  representation. Returns the string at the memory
 *  address str. The memory should be allocated on the 
 *  stack or heap before calling this function. A 
 *  maximum of 5 characters can be outputted (66535).
 *
 *  @param the string location to put the result.
 *  @param the integer to convert
 *  @return a status code of type prof_status_t.
 */
prof_status_t prof_itoa(uint8_t *str, uint16_t data);

/** @brief Begin profiler
 *
 *  This function sets up the timer before profiling.
 *  This is used by the main profiler macro.
 *
 *  @return a status code of type prof_status_t.
 */
prof_status_t prof_start();

/** @brief End profiler.
 *
 *  This function stops the profiler timer and reports 
 *  the time taken. This can be called with a message
 *  that will be passed to the logger. This needs to
 *  be terminated with "\0" like the log messages.
 *
 *  @return a status code of type prof_status_t.
 */
prof_status_t prof_stop();
#endif

/**************************************
 * @name Public functions
 */

#ifdef __PROF
/** @brief Initialize profiler
 *
 *  This function initializes the profiler. It should
 *  be called before any profiler functions. The underlying
 *  timer is initialized in this function.
 *
 *  @return a status code of type prof_status_t
 */
prof_status_t prof_Init();
#endif

/** @brief The main profile function
 *
 *  This macro implements a function that will profile
 *  the piece of code it surrounds. If the compiler
 *  directive __PROF is not set, the macro defaults to
 *  just executing the piece of code.
 *
 *  The macro just wraps the code segment under test with
 *  the prof_start() and prof_stop() function defined 
 *  earlier.
 *
 *  Example usage:
 *      For a single function or statement
 *      prof_Profile(xxx_functionToProfile());
 *      
 *      Several statements or functions
 *      prof_Profile(
 *          xxx_functionToProfile1();
 *          xxx_functionToProfile2();
 *          uint32_t someVariable = 100;
 *          // ...etc
 *      );
 *
 *  For finer control of error handling, consider calling
 *  the start and stop functions directly.
 *
 *  @return status code PROF_INFO_OK in all cases.
 */
#ifdef __PROF
#define prof_Profile(x) PROF_INFO_OK; prof_Start(); x; prof_Stop();
#else 
#define prof_Profile(x) PROF_INFO_OK; x;
#endif

#endif /* __PROF_H */
