/*
 * AT43QT_Touch.h
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */

#ifndef MAIN_USER_AT42QT_TOUCH
#define MAIN_USER_AT42QT_TOUCH

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "../../Common.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
typedef struct
{
    uint8_t guard : 1;
    uint8_t reset : 1;
    uint8_t eeprom : 1;
    uint8_t change : 1;

    uint8_t error : 1;
    uint8_t cycle : 1;
    uint8_t detect : 1;
    uint8_t one : 1;
} DeviceStatus_t;

typedef enum
{
    kTouchButtonRelease = 0,
    kTouchButtonPress,
} e_touch_buttons_action;
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef void (*touch_buttons_handler)(uint8_t, uint8_t, void *);

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
    kEraseEeprom = 0xc,
    kRecoverEeprom = 0xd,
    kCalibrateKeyN = 0x10,

    // Report
    kSendFirstKey = 0xC0,
    kSendAllKeys = 0xC1,
    kDeviceStatusCmd = 0xC2,
    kEepromCrc = 0xC3,
    kRamCrc = 0xC4,
    kErrorKeys = 0xC5,
    kSignalKeyN = 0x20,
    kRefKeyN = 0x40,
    kStatusKeyN = 0x80,
    kWriteCmd = 0x90,
    kDetectOutputStates = 0xC6,
    kLastCommand = 0xC7,
    kGetSetups = 0xC8,
    kGetDeviceId = 0xC9,
    kGetFirmwareVer = 0xCA
};

enum QT1110HardCodedSetup
{
    kKEY_AC = 1,
    kSIGNAL = 1,
    kSYNC = 1,
    kREPEAT_TIME = 2, //8,

    kQUICK_SPI = 0,
    kCHG = 1,
    kCRC = 0
};
/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/
void AT42QT_1_init(void);

void AT42QT_1_process(void);

void AT42QT_1_set_callback(touch_buttons_handler cb);
#endif /* MAIN_USER_AT42QT_TOUCH */
