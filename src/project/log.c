/** @file log.c
 *  @brief Implemenation of the log functions.
 *
 *  This contains the implementations of the 
 *  log functions.
 *
 *  @author Ben Heberlein
 *  @bug No known bugs.
 */

/*************************************
 * Includes and definitions
 */

#include "stm32f4xx.h"
#include <stdio.h>
#include "log.h"
#include "err.h"
#include <stdint.h>
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

#define LOG_MAXMSGSIZE 255
#define LOG_MAXDATASIZE 16777216
#define LOG_BAUDRATE 28800

/**************************************
 * Private functions
 */

log_status_t log_log2(log_module_t module, gen_status_t status) {
    return log_log5(module, status, NULL, 0, NULL);
}

log_status_t log_log3(log_module_t module, gen_status_t status, char *msg) {
    return log_log5(module, status, msg, 0, NULL);
}

log_status_t log_log4(log_module_t module, gen_status_t status, uint32_t len, uint8_t *data) {
    return log_log5(module, status, NULL, len, data);
}

log_status_t log_log5(log_module_t module, gen_status_t status, char *msg, uint32_t len, uint8_t *data) {
  
    // Check for data size  
    if (len > LOG_MAXDATASIZE) {
        return LOG_ERR_DATASIZE;
    }
   
    // Get msg length
    uint8_t msglen = 0;
    while (*(msg+msglen) != '\0') {
        msglen++;

        // Check for message size
        if (msglen == LOG_MAXMSGSIZE) {
            log_Log(LOG, LOG_ERR_MSGSIZE);
            return LOG_ERR_MSGSIZE;
        }
    }

    // Construct packet
    log_packet_t log_packet;
    log_packet.log_mod = module;
    log_packet.log_status = status;
    log_packet.log_msglen = msglen;
    log_packet.log_msg = (uint8_t *) msg;
    log_packet.log_datalen = len;
    log_packet.log_data = data;

    return log_send(&log_packet);
}

log_status_t log_send(log_packet_t *log_packet) {

    // Figure out sizes
    uint8_t i = 0;
    uint8_t first_size = sizeof(log_packet->log_mod) + 
                         sizeof(log_packet->log_status) + 
                         sizeof(log_packet->log_msglen);
    uint16_t second_size = log_packet->log_msglen;
    uint16_t third_size = sizeof(log_packet->log_datalen) +
                          sizeof(log_packet->log_msg) +
                          first_size;
    uint32_t fourth_size = log_packet->log_datalen;;
   
    // Send data serially
    while (i < first_size) {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {}
        USART_SendData(USART2, *(((uint8_t *)log_packet)+i));
        i++;
    }
    i = 0;
    while (i < second_size) {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {}
        USART_SendData(USART2, *(log_packet->log_msg+i));
        i++;
    }
    i = first_size + sizeof(log_packet->log_msg);
    while (i < third_size) {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {}
        USART_SendData(USART2, *(((uint8_t *)log_packet)+i));
        i++;
    }
    i = 0;
    while (i < fourth_size) {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {}
        USART_SendData(USART2, *(log_packet->log_data+i));
        i++;
    }

    return LOG_INFO_OK;

}

/**************************************
 * Public functions
 */

log_status_t log_Init() {

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // Enable GPIO clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    // Enable USART3 Clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // Connect Pin D5 to USART2 Tx
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART3);

    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Medium_Speed;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = LOG_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;

    /* USART configuration */
    USART_Init(USART2, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(USART2, ENABLE);

    return LOG_INFO_OK;
}

// Public macro function
// log_Log(...) {}
