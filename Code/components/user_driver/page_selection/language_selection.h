/*
 * language_selection.h
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */

#ifndef MAIN_USER_DRIVER_LANGUAGE_SELECTION
#define MAIN_USER_DRIVER_LANGUAGE_SELECTION

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
 * Function Name: language_gpio_init
 * Description  : initialize GPIO input for language, mode and page
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void language_gpio_init(void);

/***********************************************************************************************************************
 * Function Name: e_MODE_SELECTOR get_current_languge(void)
 * Description  : get current language
 * Arguments    : none
 * Return Value : e_LANGUAGE_SELECTOR
 ***********************************************************************************************************************/
e_LANGUAGE_SELECTOR get_current_languge(void);

#endif /* MAIN_USER_DRIVER_LANGUAGE_SELECTION */
