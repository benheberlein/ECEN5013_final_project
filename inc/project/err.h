/** @file err.h
 *  @brief Definitions for errors,
 *  warnings, and status.
 *
 *  This contains the definitions for errors, warnings, and
 *  status for every module. INFO enum values are reserved 
 *  as 0-19, WARN enum values are reserved as 20-39, and
 *  ERR enum values are reserved as 40-59.
 *
 *  This file also implementst 
 *
 *  @author Ben Heberlein
 *  @bug No known bugs.
 */

#ifndef __ERR_H
#define __ERR_H

/**************************************
 * @name Includes and definitions
 */

#include <stdint.h>

/* @brief Error sections, INFO is 0-19,
 * WARN is 20-39, ERR is 40-59
 */
#define INFO 0
#define WARN 20
#define ERR  40
#define END  60
/* @brief General status type for module status typecast.
 */
typedef uint8_t gen_status_t;

/* @brief Log module status
 */
typedef enum log_status_e {
    LOG_INFO_OK = INFO,
    LOG_WARN_UNKNOWN = WARN,
    LOG_ERR_DATASIZE = ERR,
    LOG_ERR_MSGSIZE = ERR+1,
    LOG_ERR_UNKNOWN = END-1
} log_status_t;

/* @brief CMD module status
 */
typedef enum cmd_status_e {
    CMD_INFO_OK = INFO,
    CMD_ERR_UNKNOWN = END,
    //...
} cmd_status_t;

/* @brief STDLIB status
 */
typedef enum stdlib_status_e {
    STDLIB_INFO_OK = INFO,
    STDLIB_ERR_UNKNOWN = END,
} stdlib_status_e;

/**************************************
 * @name Public functions
 */

#ifdef  USE_FULL_ASSERT
/**
 * @brief Reports the name of the source file and the 
 * source line number where the assert_param error has
 * occured. Used by the STM32f4xx standard peripheral
 * library to report issues
 *
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line) { 
    /* User can add his own implementation to report the file name and line number,
     * ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1) {}
}
#endif

# endif /* __ERR_H */
