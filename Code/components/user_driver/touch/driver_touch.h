/*
 * page_selection.h
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */

#ifndef MAIN_USER_DRIVER_DRIVER_TOUCH
#define MAIN_USER_DRIVER_DRIVER_TOUCH

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

enum QT1110Misc
{
    kOK = 0x55,
    kDeviceId = 0x57,
    kDeviceCommDelay = 150
};

enum QT1110Commands
{
// Control
    kSendSetups = 0x1,
    kCalibrateAll = 0x3,
    kChipReset = 0x4,
    kSleep = 0x5,
    kSaveToEeprom = 0xA,
    kLoadFromEeprom = 0xB,
    kEraseEeprom    = 0xc,
    kRecoverEeprom  = 0xd,
    kCalibrateKeyN   = 0x10,

    // Report
    kSendFirstKey = 0xC0,
    kSendAllKeys  = 0xC1,
    kDeviceStatusCmd = 0xC2,
    kEepromCrc    = 0xC3,
    kRamCrc       = 0xC4,
    kErrorKeys    = 0xC5,
    kSignalKeyN   = 0x20,
    kRefKeyN      = 0x40,
    kStatusKeyN   = 0x80,
    kWriteCmd     = 0x90,
    kDetectOutputStates = 0xC6,
    kLastCommand  = 0xC7,
    kGetSetups    = 0xC8,
    kGetDeviceId     = 0xC9,
    kGetFirmwareVer  = 0xCA
};


enum QT1110HardCodedSetup
{
    kKEY_AC      = 1,
    kSIGNAL      = 1,
    kSYNC        = 1,
    kREPEAT_TIME = 2,//8,

    kQUICK_SPI   = 0,
    kCHG         = 1,
    kCRC         = 0
};
/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/

/***********************************************************************************************************************
 * Function Name: e_PAGE_SELECTOR page_selection_detect
 * Description  : get current page
 * Arguments    : none
 * Return Value : e_PAGE_SELECTOR
 ***********************************************************************************************************************/

#endif /* MAIN_USER_DRIVER_DRIVER_TOUCH */
