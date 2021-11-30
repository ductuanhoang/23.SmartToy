/*
 * user_spi.h
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */

#ifndef MAIN_USER_SPI
#define MAIN_USER_SPI

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "../../Common.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/
/***********************************************************************************************************************
 * Function Name: user_spi_init
 * Description  : support initialization two different spi devices
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void user_spi_init(uint8_t spi_number);

/***********************************************************************************************************************
 * Function Name: spi_touch1_cs_pause
 * Description  : pause communication touch 1
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void spi_touch1_cs_pause(void);
void spi_touch1_cs_resume(void);
void spi_touch1_write(uint8_t value);
esp_err_t spi_touch1_read(uint8_t regAddr, uint8_t* data, uint8_t length);

void spi_touch2_cs_pause(void);
void spi_touch2_cs_resume(void);
void spi_touch2_write(uint8_t value);
esp_err_t spi_touch2_read(uint8_t regAddr, uint8_t* data, uint8_t length);


#endif /* MAIN_USER_SPI */
