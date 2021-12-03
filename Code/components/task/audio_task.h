
/*
 * audio_task.h
 *
 *  Created on: Nov 24, 2020
 *      Author: Yolo
 */
#ifndef MAIN_TASK_AUDIO_TASK_H_
#define MAIN_TASK_AUDIO_TASK_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "../../Common.h"
#include "../user_driver/play_audio/play_audio.h"
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
 * Function Name:
 * Description  : call in loop to process peripheral
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void audio_task(void);

void audio_task_activate_callback(E_PLAY_AUDIO_TYPE _type,
                             e_MODE_SELECTOR _mode,
                             e_LANGUAGE_SELECTOR _language,
                             e_PAGE_NUMBER _page_number,
                             e_TOUCH_NUMBER _touch_number);
#endif /* MAIN_TASK_AUDIO_TASK_H_ */
