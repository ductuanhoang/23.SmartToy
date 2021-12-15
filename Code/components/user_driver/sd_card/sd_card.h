/*
 * sd_card.h
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */

#ifndef MAIN_USER_DRIVER_SD_CARD_H_
#define MAIN_USER_DRIVER_SD_CARD_H_

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
* Function Name: user_sd_card_init
* Description  : 
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void user_sd_card_init(void);

void user_sd_card_test(void);

void user_play_audio(const char *fname);
#endif /* MAIN_USER_DRIVER_SD_CARD_H_ */
