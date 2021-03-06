/*
 * play_audio_sd_card.h
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */

#ifndef MAIN_USER_DRIVER_PLAY_AUDIO_SD_CARD
#define MAIN_USER_DRIVER_PLAY_AUDIO_SD_CARD

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
 * Function Name: sd_card_play_file
 * Description  : 
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void sd_card_play_file(char *file_path);
#endif /* MAIN_USER_DRIVER_PLAY_AUDIO_SD_CARD */
