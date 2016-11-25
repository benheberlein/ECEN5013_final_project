/** @file cmd.c
 *  @brief Implemenation of the command module.
 *
 *  This contains the implementations of the 
 *  command module functions.
 *
 *  @author Ben Heberlein
 *  @bug No known bugs.
 */

/*************************************
 * Includes and definitions
 */

#include "cmd.h"
#include "err.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include <stdint.h>
#include <stdlib.h>

#ifdef __CMD
#include "stm32f4xx_usart.h"
#endif

/* @brief instance of queue
 */
static cmd_queue_t *cmd_queue;

#ifdef __CMD
/* @brief UART2 Rx cmd buffer
 */
static cmd_cmd_t *cmd_uartBuf;

/* @brief UART2 Rx counter
 */
static uint32_t cmd_uartCtr;

/* @brief UART2 Rx total size
 */
static uint32_t cmd_uartTotal;

/* @brief UART2 Rx data size
 */
static uint16_t cmd_uartDataSize;

/* @brief UART2 Rx data start
 */
#define CMD_DATASTART 4

/* @brief UART2 Rx data len start
 */
#define CMD_DATALENSTART 2
#endif

/**************************************
 * Private functions
 */

cmd_status_t cmd_queueInit() {

    // Allocate space for queue
    cmd_queue = NULL;
    cmd_queue = (cmd_queue_t *) malloc(sizeof(cmd_queue_t));

    if (cmd_queue == NULL) {
        log_Log(CMD, CMD_ERR_MALLOC, "Couldn't initialize cmd_queue memory.\0");
        return CMD_ERR_MALLOC;
    } 

    cmd_queue->cmd_queue_buf = NULL;
    cmd_queue->cmd_queue_buf = (cmd_cmd_t **) malloc(sizeof(cmd_cmd_t *)*CMD_QUEUE_CAP);
    if (cmd_queue->cmd_queue_buf == NULL) {
        log_Log(CMD, CMD_ERR_MALLOC, "Couldn't initialize cmd_queue_buf memory.\0");
        return CMD_ERR_MALLOC;
    } 

    cmd_queue->cmd_queue_head = cmd_queue->cmd_queue_buf;
    cmd_queue->cmd_queue_tail = cmd_queue->cmd_queue_buf;
    cmd_queue->cmd_queue_capacity = CMD_QUEUE_CAP;
    cmd_queue->cmd_queue_size = 0;
    cmd_queue->cmd_queue_status = CMD_QUEUE_EMPTY;

    return CMD_INFO_OK;
}

#ifdef __CMD
cmd_status_t cmd_uartInit() {

    // Initialize command buffer
    cmd_uartBuf = NULL;
    cmd_uartBuf = (cmd_cmd_t *) malloc(sizeof(cmd_cmd_t));
    
    // Check for failure
    if (cmd_uartBuf == NULL) {
        log_Log(CMD, CMD_ERR_MALLOC, "Couldn't initialize cmd_uartBuf memory\0");
        return CMD_ERR_MALLOC;
    }

    // Set to initial values
    cmd_uartBuf->cmd_module = 0;
    cmd_uartBuf->cmd_func = 0;
    cmd_uartBuf->cmd_dataLen = 0;
    cmd_uartBuf->cmd_data = NULL;
    cmd_uartCtr = 0;
    cmd_uartTotal = CMD_DATASTART;
    cmd_uartDataSize = 0;

    // Structures for configuring
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // Enable GPIO clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    // Enable USART2 Clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // Connect Pin D5 to USART2 Rx
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);
    
    // Configure USART Rx as alternate function
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = CMD_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = (USART2->CR1 & (USART_CR1_RE | USART_CR1_TE)) | USART_Mode_Rx;

    // USART configuration
    USART_Init(USART2, &USART_InitStructure);

    // Enable Rx interrupts
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    // Initialize interrupts
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable USART2
    USART_Cmd(USART2, ENABLE);
    
    return CMD_INFO_OK;

}

void USART2_IRQHandler(void) {
    if (USART_GetITStatus(USART2, USART_IT_RXNE)) {
        uint8_t ch = USART2->DR;      

        if (cmd_uartCtr >= CMD_DATASTART) {
            // Save character to data buffer
            *(cmd_uartBuf->cmd_data + cmd_uartCtr - CMD_DATASTART) = ch;
        } else {
            // Save character to buffer
            *((uint8_t *)(cmd_uartBuf) + cmd_uartCtr) = ch;
        }
        // Get the first byte of data length
        if (cmd_uartCtr == CMD_DATALENSTART) {
            cmd_uartDataSize += ch;
        // Get the second byte
        } else if (cmd_uartCtr == CMD_DATALENSTART + 1) {
            cmd_uartDataSize += ch << 8;
            // We're done if no data
            if (cmd_uartDataSize != 0) {
                cmd_uartBuf->cmd_data = (uint8_t *) malloc(cmd_uartDataSize);
                cmd_uartTotal += cmd_uartDataSize;
            }
        }

        // Increase and check if we're done
        cmd_uartCtr++;        
        if (cmd_uartCtr >= cmd_uartTotal) {
            cmd_uartCtr = 0;
            cmd_uartTotal = CMD_DATASTART;
            cmd_uartDataSize = 0;
            cmd_status_t st = cmd_QueueGetStatus();
                // Check if there's room
            if (st != CMD_INFO_QUEUEEMPTY && st != CMD_INFO_QUEUEPARTIAL) {
                log_Log(CMD, st, "Bad queue state. Could not write command to queue.\0");
            } else {
                // Copy queue buffer to new array
                cmd_cmd_t *cmdCopy;
                st = cmd_CmdAllocate(&cmdCopy, cmd_uartBuf->cmd_dataLen);
                if (st != CMD_INFO_OK) {
                    log_Log(CMD, st, "Could not copy command. Could not write command to queue.\0");
                }
                cmdCopy->cmd_module = cmd_uartBuf->cmd_module;
                cmdCopy->cmd_func = cmd_uartBuf->cmd_func;
                cmdCopy->cmd_dataLen = cmd_uartBuf->cmd_dataLen;

                // Don't need the data that CmdAllocate made for this
                free(cmdCopy->cmd_data);
                cmdCopy->cmd_data = cmd_uartBuf->cmd_data;
              
                // Add command to queue
                st = cmd_QueuePut(cmdCopy);
                if (st != CMD_INFO_OK) {
                    log_Log(CMD, st, "Could not add command to queue.\0");
                }

                // Zero out cmd buffer
                cmd_uartBuf->cmd_module = 0;
                cmd_uartBuf->cmd_func = 0;
                cmd_uartBuf->cmd_dataLen = 0;
                cmd_uartBuf->cmd_data = NULL;               
            }         
        }
    }
}

#endif

/**************************************
 * Public functions
 */

cmd_status_t cmd_CmdDeallocate(cmd_cmd_t *cmd) {
    // Check for null issues
    if (cmd == NULL) {
        log_Log(CMD, CMD_WARN_FREE, "Attempting to free memory that has already been freed.\0");
        return CMD_WARN_FREE;
    }
    if (cmd->cmd_data == NULL) {
        free(cmd);
        log_Log(CMD, CMD_WARN_FREE, "Attempting to free memory that has already been freed.\0");
        return CMD_WARN_FREE;
    }
    
    // Free the data
    free(cmd->cmd_data);
    free(cmd);

    return CMD_INFO_OK;
}

cmd_status_t cmd_CmdAllocate(cmd_cmd_t **cmd, uint16_t dataLen) {     
    if (cmd == NULL) {
        log_Log(CMD, CMD_ERR_NULLPTR, "cmd_CmdAllocate parameter cannot be NULL.\0");
        return CMD_ERR_NULLPTR;
    }

    // Allocate struct
    *cmd = NULL;
    *cmd = (cmd_cmd_t *) malloc(sizeof(cmd_cmd_t));
    if (*cmd == NULL) {
        log_Log(CMD, CMD_ERR_MALLOC, "Could not allocate command structure.\0");
        return CMD_ERR_MALLOC;
    }

    // Allocate data in struct
    (*cmd)->cmd_data = NULL;
    (*cmd)->cmd_data = (uint8_t *) malloc(dataLen);
    if ((*cmd)->cmd_data == NULL) {
        log_Log(CMD, CMD_ERR_MALLOC, "Could not allocate command data.\0");
        return CMD_ERR_MALLOC;
    }

    // Set to defaults
    (*cmd)->cmd_module = 0;
    (*cmd)->cmd_func = 0;
    (*cmd)->cmd_dataLen = dataLen;

    return CMD_INFO_OK;
}

cmd_status_t cmd_QueueGetStatus() {
    switch (cmd_queue->cmd_queue_status) {
        case CMD_QUEUE_EMPTY:
            return CMD_INFO_QUEUEEMPTY;
        case CMD_QUEUE_FULL:
            return CMD_INFO_QUEUEFULL;
        case CMD_QUEUE_PARTIAL:
            return CMD_INFO_QUEUEPARTIAL;
        default:
            return CMD_ERR_QUEUEINVALID;
    }    
}

cmd_status_t cmd_QueuePut(cmd_cmd_t *cmd) {
    // Check for full queue
    if (cmd_queue->cmd_queue_status == CMD_QUEUE_FULL) {
        log_Log(CMD, CMD_ERR_QUEUEFULL);
        return CMD_ERR_QUEUEFULL;
    }
    if (cmd_queue->cmd_queue_status == CMD_QUEUE_INVALID) {
        log_Log(CMD, CMD_ERR_QUEUEINVALID);
        return CMD_ERR_QUEUEINVALID;
    }

    // Set data
    *cmd_queue->cmd_queue_head = cmd;

    // Increment head and check for buffer wrap
    cmd_queue->cmd_queue_head++;
    if ((cmd_queue->cmd_queue_head - cmd_queue->cmd_queue_buf) >= 
         cmd_queue->cmd_queue_capacity) {

        cmd_queue->cmd_queue_head -= cmd_queue->cmd_queue_capacity;
    }
    cmd_queue->cmd_queue_size++;

    // Set new state
    if (cmd_queue->cmd_queue_size == cmd_queue->cmd_queue_capacity ||
        cmd_queue->cmd_queue_head == cmd_queue->cmd_queue_tail) {

        cmd_queue->cmd_queue_status = CMD_QUEUE_FULL;
    } else {
        cmd_queue->cmd_queue_status = CMD_QUEUE_PARTIAL;
    }

    return CMD_INFO_OK;
}

cmd_status_t cmd_QueueGet(cmd_cmd_t **cmd) {
    // Check for valud cmd pointer
    if (cmd == NULL) {
        log_Log(CMD, CMD_ERR_NULLPTR, "Invalid command pointer in parameter list for cmd_QueueGet.\0");
        return CMD_ERR_NULLPTR;
    }

    // Check if queue is empty
    if (cmd_queue->cmd_queue_status == CMD_QUEUE_EMPTY) {
        log_Log(CMD, CMD_ERR_QUEUEEMPTY, "Trying to get from an empty command queue.\0");
        return CMD_ERR_QUEUEEMPTY;
    }

    // Increment tail and check for wrap
    cmd_queue->cmd_queue_tail++;
    if ((cmd_queue->cmd_queue_tail - cmd_queue->cmd_queue_buf) >=
         cmd_queue->cmd_queue_capacity) {

        cmd_queue->cmd_queue_tail -= cmd_queue->cmd_queue_capacity;
    }
    cmd_queue->cmd_queue_size--;

    // Set new state
    if (cmd_queue->cmd_queue_size == 0) {
        cmd_queue->cmd_queue_status = CMD_QUEUE_EMPTY;
    } else {
        cmd_queue->cmd_queue_status = CMD_QUEUE_PARTIAL;
    }

    return CMD_INFO_OK;
}

cmd_status_t cmd_Init() {
#ifdef __CMD
    cmd_status_t ret = cmd_queueInit();
    if (ret != CMD_INFO_OK) {
        return ret;
    }
    return cmd_uartInit();
#else 
    return cmd_queueInit();
#endif
}

