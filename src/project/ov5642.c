/** @file ov5642.c
 *  @brief Implemenation of the ov5642 camera functions.
 *
 *  This file implements functionality of the ov5642
 *  camera module. For function descriptions, see the 
 *  ov5642.h file in the include directory.
 *
 *  @author Ben Heberlein
 *  @bug No known bugs.
 */

/*************************************
 * Includes and definitions
 */

#include "ov5642.h"
#include "ov5642_regs.h"
#include "sdram.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_dcmi.h"

#include <stdint.h>

/**************************************
 * Private functions
 */

ov5642_status_t ov5642_clockInit() {
    GPIO_InitTypeDef gpioInit;

    // Configure for HSI clock, no prescaler
    RCC_MCO1Config(RCC_MCO1Source_HSI, RCC_MCO1Div_1);

    // Enable GPIO clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // Map alternate function 
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);

    // Configure PA8
    gpioInit.GPIO_OType = GPIO_OType_PP;
    gpioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpioInit.GPIO_Mode = GPIO_Mode_AF;
    gpioInit.GPIO_Pin = GPIO_Pin_8;
    gpioInit.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOA, &gpioInit);

    return OV5642_INFO_OK;
}

ov5642_status_t ov5642_dmaInit() {
    DMA_InitTypeDef dmaInit;
    
    /* We are going to use DMA2, Stream 1, Channel 1
     * to handle DCMI DMA requests. See page 310 of
     * the STM32F4 family reference manual RM0090. */

    // Enable clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    // Deinit existing stream
    DMA_Cmd(DMA2_Stream1, DISABLE);
    DMA_DeInit(DMA2_Stream1);

    // Construct initialization config
    dmaInit.DMA_Channel = DMA_Channel_1;
    dmaInit.DMA_PeripheralBaseAddr = OV5642_DCMI_PERIPHADDR;
    dmaInit.DMA_Memory0BaseAddr = SDRAM_BASEADDR;
    dmaInit.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dmaInit.DMA_BufferSize = OV5642_DMA_BUFSIZE;
    dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dmaInit.DMA_Mode = DMA_Mode_Circular;
    dmaInit.DMA_Priority = DMA_Priority_High;
    dmaInit.DMA_FIFOMode = DMA_FIFOMode_Enable;
    dmaInit.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    dmaInit.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    // Initialize
    DMA_Init(DMA2_Stream1, &dmaInit);

    // Enable
    DMA_Cmd(DMA2_Stream1, ENABLE);

    return OV5642_INFO_OK;
}

ov5642_status_t ov5642_dcmiInit() {
    GPIO_InitTypeDef gpioInit;

    /**********************************
     * DCMI     | Pin
     * --------------------------------
     * VSYNC    | PB7
     * HSYNC    | PA4
     * PIXCLK   | PA6
     * D7       | PB9
     * D6       | PB8
     * D5       | PD3
     * D4       | PC11
     * D3       | PC9
     * D2       | PC8
     * D1       | PC7
     * D0       | PC6
     **********************************/

    // Enable GPIO clocks
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
                           RCC_AHB1Periph_GPIOB |
                           RCC_AHB1Periph_GPIOC |
                           RCC_AHB1Periph_GPIOD, 
                           ENABLE);
    // Port A
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_DCMI); // HSYNC
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_DCMI); // PIXCLK

    gpioInit.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
    gpioInit.GPIO_Mode = GPIO_Mode_AF;
    gpioInit.GPIO_Speed = GPIO_High_Speed;
    gpioInit.GPIO_OType = GPIO_OType_PP;
    gpioInit.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_Init(GPIOA, &gpioInit); 

    // Port B
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_DCMI); // VSYNC
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_DCMI); // D6
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_DCMI); // D7

    gpioInit.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    gpioInit.GPIO_Mode = GPIO_Mode_AF;
    gpioInit.GPIO_Speed = GPIO_High_Speed;
    gpioInit.GPIO_OType = GPIO_OType_PP;
    gpioInit.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_Init(GPIOB, &gpioInit); 

    // Port C
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_DCMI);  // D0
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_DCMI);  // D1
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_DCMI);  // D2
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_DCMI);  // D3
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_DCMI); // D4
    
    gpioInit.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 |
                        GPIO_Pin_8 | GPIO_Pin_9 |
                        GPIO_Pin_11;
    gpioInit.GPIO_Mode = GPIO_Mode_AF;
    gpioInit.GPIO_Speed = GPIO_High_Speed;
    gpioInit.GPIO_OType = GPIO_OType_PP;
    gpioInit.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_Init(GPIOC, &gpioInit); 

    // Port D
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_DCMI); // D5

    gpioInit.GPIO_Pin = GPIO_Pin_3;
    gpioInit.GPIO_Mode = GPIO_Mode_AF;
    gpioInit.GPIO_Speed = GPIO_High_Speed;
    gpioInit.GPIO_OType = GPIO_OType_PP;
    gpioInit.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_Init(GPIOA, &gpioInit); 

    // Initialize DCMI
    DCMI_InitTypeDef dcmiInit;

    // Send clock to DCMI
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);

    // DCMI Init
    dcmiInit.DCMI_CaptureMode = DCMI_CaptureMode_SnapShot;
    dcmiInit.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
    dcmiInit.DCMI_PCKPolarity = DCMI_PCKPolarity_Falling;
    dcmiInit.DCMI_VSPolarity = DCMI_VSPolarity_High;
    dcmiInit.DCMI_HSPolarity = DCMI_HSPolarity_High;
    dcmiInit.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;
    dcmiInit.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;

    // Initialize
    DCMI_Init(&dcmiInit);

    // Turn on JPEG mode
    DCMI_JPEGCmd(ENABLE);

    // Enable interrupt on frame complete in DCMI
    DCMI_ITConfig(DCMI_IT_FRAME, ENABLE);

    // NVIC Enable interrupt on frame complete
    NVIC_InitTypeDef nvicInit;
    nvicInit.NVIC_IRQChannel = DCMI_IRQn;
    nvicInit.NVIC_IRQChannelPreemptionPriority = 0;
    nvicInit.NVIC_IRQChannelSubPriority = 0;
    nvicInit.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInit);

    // Enable DCMI
    DCMI_Cmd(ENABLE);

    return OV5642_INFO_OK;
}

ov5642_status_t ov5642_i2cInit() {
    return OV5642_INFO_OK;
}

void DCMI_IRQHandler() {
    return;
}

/**************************************
 * Public functions
 */

ov5642_status_t ov5642_Init() {
    return OV5642_INFO_OK;
}

ov5642_status_t ov5642_ConfigTransmit() {
    return OV5642_INFO_OK;
}

ov5642_status_t ov5642_Capture() {
    DCMI_CaptureCmd(ENABLE);
    return OV5642_INFO_OK;
}
