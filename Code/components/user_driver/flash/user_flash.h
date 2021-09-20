/*
 * user_flash.h
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */

#ifndef MAIN_USER_DRIVER_USER_FLASH_SENSOR_H_
#define MAIN_USER_DRIVER_USER_FLASH_SENSOR_H_

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
void flash_file_init(void);

bool logger_list_file(char *location);
uint8_t check_map_size(void);

void flash_erase_all_partions(void);
#endif /* MAIN_USER_DRIVER_USER_FLASH_SENSOR_H_ */
