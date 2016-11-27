/** @file ov5642.h
 *  @brief Function prototypes for the ov5642 camera
 *
 *  This contains the driver function prototypes, macros,
 *  constants, and global variables.
 *
 *  @author Ben Heberlein
 *  @bug No known bugs.
 */

#ifndef __OV5642_H
#define __OV5642_H

/*************************************
 * @name Includes and definitions
 */

#include "err.h"
#include <stdint.h>

/* @brief Base address of DCMI module and offset of DR 
 * register for DMA requests.
 */
#define OV5642_DCMI_BASEADDR ((uint32_t)0x50050000)
#define OV5642_DCMI_OFFSETDR ((uint32_t)0x28)
#define OV5642_DCMI_PERIPHADDR (OV5642_DCMI_BASEADDR | OV5642_DCMI_OFFSETDR)

/* @brief Transfer size
 */
#define OV5642_DMA_BUFSIZE ((uint16_t)65535)

/**************************************
 * @name Private functions
 */

/** @brief Initialize the clock with RCC.
 *
 *  This function initializes the clock used by the 
 *  ov5642 camera module.
 *
 *  @return a status code of the type ov5642_status_t
 */
ov5642_status_t ov5642_clockInit();

/** @brief Initialize the GPIO pins for DCMI and I2C
 *
 *  This function initializes the GPIO used by the 
 *  ov5642 camera module.
 *
 *  @return a status code of the type ov5642_status_t
 */
ov5642_status_t ov5642_gpioInit();

/** @brief Initialize the DMA controller
 *
 *  This function initializes and configures DMA for the 
 *  ov5642 camera module. DMA is configured with DMA2,
 *  Stream 1, Channel 1. See page 310 of the STM32F4 
 *  family reference manual RM0090.
 *
 *  Configured for circular mode, with maximum stream NDT
 *  transfer size. Configured to transfer into SDRAM.
 *
 *  @return a status code of the type ov5642_status_t
 */
ov5642_status_t ov5642_dmaInit();

/** @brief Initialize the DCMI module
 *
 *  This function initializes and configures DCMI. The 
 *  following pin mapping is used.
 *
 *  DCMI     | Pin
 *  --------------------------------
 *  VSYNC    | PB7
 *  HSYNC    | PA4
 *  PIXCLK   | PA6
 *  D7       | PB9
 *  D6       | PB8
 *  D5       | PD3
 *  D4       | PC11
 *  D3       | PC9
 *  D2       | PC8
 *  D1       | PC7
 *  D0       | PC6
 *  -----------------------------------
 *
 *  DCMI is configured for snapshot mode.
 *
 *  @return a status code of the type ov5642_status_t
 */
ov5642_status_t ov5642_dcmiInit();

/** @brief Initialize I2C
 *
 *  This function initializes I2C to set and control 
 *  ov5642 camera configuration.
 *
 *  @return a status code of the type ov5642_status_t
 */
ov5642_status_t ov5642_i2cInit();

/** @brief Frame complete interrupt handler
 *
 *  This file implements an interrupt handler for the
 *  frame complete interrupt in the DCMI controller.
 *  Currently this handler does nothing.
 */ 
void DCMI_IRWHandler();

/**************************************
 * @name Public functions
 */

/** @brief Initialize the ov5642 camera
 *
 *  This function fully initializes the ov5642 camera.
 *  This should be called before attempting to configure
 *  or take an image.
 *
 *  @return a status code of the type ov5642_status_t
 */
ov5642_status_t ov5642_Init();

/** @brief Transfer a configuration to the camera over I2C
 *
 *  This function transfers a configuration from the 
 *  definitions in ov5642_regs.h to the camera module 
 *  over I2C.
 *
 *  @return a status code of the type ov5642_status_t
 */
ov5642_status_t ov5642_ConfigTransmit();

/** @brief Capture an image and put it into SDRAM.
 *
 *  This function commands the ov5642 module to take an
 *  image, and transfers the image to SDRAM using DMA and 
 *  DCMI functionality.
 *
 *  @return a status code of the type ov5642_status_t
 */
ov5642_status_t ov5642_Capture();

/** @brief Transfer an image from SDRAM to the host.
 *
 *  This function uses the logger to transfer a full
 *  image from SDRAM to the host computer. This function
 *  will not work if the logger is disabled (directive 
 *  __LOG needs to be on).
 *
 *  @return a status code of the type ov5642_status_t
 */
ov5642_status_t ov5642_Capture();

# endif /* __OV5642_H */
