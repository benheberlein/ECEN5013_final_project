/** @file test.h
 *  @brief Declarations for the test framework.
 *
 *  Defines several macros that handle unit testing.
 *  Test files should be in the src/test folder, with
 *  names corresponding to the module under test.
 *  For example, for unit tests of logger functions,
 *  there would be a file test_log.c in src/test.
 *
 *  Inside the module test file, there should be tests
 *  corresponding to each function under test. For example,
 *  if there were functions in log.c called
 *  log_someFunction1(), 
 *  log_someFunction2(),
 *  log_someFunction3(),
 *  we would have associated test function in test_log.c
 *  test_log_someFunction1(),
 *  test_log_someFunction2(),
 *  test_log_someFunction3(),
 *  ...etc.
 *
 *  These functions report output with the logger.
 *  Make sure both __TEST and __LOG are defined in the
 *  build system before using these functions.
 *
 *  This test framework is based off of MinUnit - a minimal
 *  unit testing framework for C, found at
 *  http://www.jera.com/techinfo/jtns/jtn002.html
 *  MinUnit is released for any purpose by the author.
 *
 *  @author Ben Heberlein
 *  @bug No known bugs.
 */

#ifndef __TEST_H_
#define __TEST_H

/*************************************
 * Includes and definitions
 */

#include <stdint.h>

/**************************************
 * Private functions
 */

/**************************************
 * Public functions
 */

#ifdef __TEST
/** @brief Asserts a condition within a test
 *  
 *  This function needs to be used inside of a test 
 *  function called by the test_Test() function. This
 *  will exit the test function with a message if the
 *  condition fails.
 * 
 *  @param test the condition to be tested
 *  @param message a message to log if ther is a failure
 *  @return exit function with message if failure
 */
#define test_Assert(condition, message) \
    do { \
        if (!(condition)) { \
            return message; \
        } \
    } while(0)

/** @brief Runs a test for an input test function
 * 
 *  This function uses the logger to log a passed or
 *  failed test. Tests should have several test_Assert()
 *  statements in them. If any of the assertions fail,
 *  the test returns TEST_INFO_FAILED and logs the 
 *  associated failure message.
 *
 *  @param test the input test function name, no params
 *  @return a status of either
 *      TEST_INFO_PASSED
 *      TEST_INFO_FAILED
 */
#define test_Test(test) \
    do { \
        uint8_t *message = test(); \
        if (message != NULL) { \
            log_Log(TEST, TEST_INFO_FAILED, message); \
            return TEST_INFO_FAILED); \
        } else \
            log_Log(TEST, TEST_INFO_PASSED); \
            return TEST_INFO_PASSED; \
        } \
    } while(0)
#endif

# endif /* __TEST_H */
