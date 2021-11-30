/*
 * driver_touch.c
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */
/***********************************************************************************************************************
 * Pragma directive
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes <System Includes>
 ***********************************************************************************************************************/
#include "AT42QT_Touch.h"
/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
typedef void (*read_callback)(uint8_t regAddr, uint8_t *data, uint8_t length);
typedef void (*write_callback)(uint8_t regAdd) typedef void (*cs_callback)(void);
/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
static unsigned long IRAM_ATTR micros()
{
    return (unsigned long)(esp_timer_get_time());
}

static void IRAM_ATTR delayMicroseconds(uint32_t us)
{
    uint32_t m = micros();
    if (us)
    {
        uint32_t e = (m + us);
        if (m > e)
        { // overflow
            while (micros() > e)
            {
                NOP();
            }
        }
        while (micros() < e)
        {
            NOP();
        }
    }
}
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/
void AT42QT_Init(void)
{
    AT42QT_Configure(11, 9);
    AT42QT_Calibrate();
}
/***********************************************************************************************************************
 * Function Name: AT42QT_GetID
 * Description  :
 * Arguments    : none
 * Return Value :
 ***********************************************************************************************************************/
bool AT42QT_GetID(uint8_t *id, read_callback read, cs_callback pause)
{
    bool status = false;
    uint8_t try_count = 0;
    uint8_t recv_data = 0x00;
    do
    {
        // delayMicroseconds(kDeviceCommDelay);
        read(kGetDeviceId, &recv_data, 1);
        if ((recv_data == kOK) | (recv_data == kGetDeviceId))
        {
            recv_data = 0x00;
            pause();
            // delayMicroseconds(kDeviceCommDelay);
            read(0x00, &recv_data, 1);
            if (recv_data == kDeviceId)
            {
                status = true;
                *id = recv_data;
            }
            else
                status = false;
        }
        else if (recv_data == kDeviceId)
        {
            status = true;
            *id = recv_data;
        }
        try_count++;
    } while ((try_count < 30) & (status == false));
    return status;
}
/***********************************************************************************************************************
 * Function Name: AT42QT_GetVer
 * Description  :
 * Arguments    : none
 * Return Value :
 ***********************************************************************************************************************/
bool AT42QT_GetVer(uint8_t *ver, read_callback read, cs_callback pause)
{
    bool status = false;
    uint8_t try_count = 0;
    uint8_t recv_data = 0x00;
    do
    {
        read(kGetFirmwareVer, &recv_data, 1);
        if (recv_data == kOK)
        {
            recv_data = 0x00;
            pause();
            delayMicroseconds(kDeviceCommDelay);
            read(0x00, &recv_data, 1);
            if ((recv_data != kOK) && (recv_data != kGetFirmwareVer))
            {
                status = true;
                *ver = recv_data;
            }
            else
                status = false;
        }
        try_count++;
    } while ((try_count < 30) & (status == false));
    return status;
}

DeviceStatus_t AT42QT_GetStatus(void)
{
    union
    {
        uint8_t c;
        DeviceStatus_t stat;
    } ret;
    bool status = AT42QT_GetCmd(kDeviceStatusCmd, &ret.c, 1);
    delayMicroseconds(kDeviceCommDelay);
    return ret.stat;
}

/***********************************************************************************************************************
 * Static Functions
 ***********************************************************************************************************************/
static uint8_t AT42QT_GetKey()
{
    uint8_t recv_data;
    AT42QT_GetCmd(kSendFirstKey, &recv_data, 1);
    if ((recv_data & 0x80) == 0)
        return 0;
    return (recv_data & 0xf) + 1;
}

static uint16_t AT42QT_GetAllKeys(void)
{
    uint16_t ret;
    delayMicroseconds(kDeviceCommDelay); // In multibyte communications, the master must pause for a minimum delay of 150 us between the completion of one byte exchange and the beginning of the next.
    AT42QT_GetCmd(kSendAllKeys, &ret, 2);
    return (ret << 1); // Shift the bitmap by one bit to
}

static void AT42QT_Configure(uint8_t numkeys, uint8_t guardkey)
{
    if (numkeys == 11)
        numkeys = 1;
    else
        numkeys = 0;

    uint8_t data = (kKEY_AC << 7) | (numkeys << 6) | (kSIGNAL << 5) | (kSYNC << 4) | (kREPEAT_TIME & 0xf);
    AT42QT_SetCmd(0, data);

    if (guardkey > 0)
        data = (((guardkey - 1) & 0xf) << 4) | (1 << 3); // set the guard key number and GD_EN
    else
        data = 0;
    data |= (kQUICK_SPI << 2) | (kCHG << 1) | (kCRC << 0);
    AT42QT_SetCmd(1, data);

    // Atmel firmware bug requires a reset (or maybe a load()) to actually switch the mode
    AT42QT_Save();
    AT42QT_Reset();
}

static void AT42QT_Save(void)
{
    delayMicroseconds(kDeviceCommDelay);
    spiWriteVal(kSaveToEeprom);
    vTaskDelay(150 / portTICK_RATE_MS);
}

static void AT42QT_Reset(void)
{
    spiWriteVal(kChipReset);
    vTaskDelay(150 / portTICK_RATE_MS); // after reset device need 150ms
}

static void AT42QT_Calibrate(void)
{
    delayMicroseconds(kDeviceCommDelay);
    spiWriteVal(kCalibrateAll);
    vTaskDelay(150 / portTICK_RATE_MS); // after reset device need 150ms
}

static void AT42QT_Erase(void)
{
    spiWriteVal(kEraseEeprom);
    vTaskDelay(150 / portTICK_RATE_MS); // after reset device need 150ms
}

static bool AT42QT_GetCmd(uint8_t cmd, uint8_t *data, uint8_t length)
{
    bool status = false;
    uint8_t try_count = 0;
    do
    {
        spiReadVal(cmd, data, 1);
        if ((data[0] == kOK) | (data[0] == cmd))
        {
            pause();
            delayMicroseconds(kDeviceCommDelay);
            spiReadVal(0x00, data, length);
            if ((data[0] != kOK) && (data[0] != cmd))
            {
                status = true;
            }
            else
                status = false;
        }
        try_count++;
    } while ((try_count < 30) & (status == false));
    return status;
}

static bool AT42QT_SetCmd(uint8_t addr, uint8_t data)
{
    if (addr > 41)
    {
        return false;
    }
    delayMicroseconds(kDeviceCommDelay);
    spiWriteVal(kWriteCmd + addr);
    delayMicroseconds(kDeviceCommDelay);
    spiWriteVal(data);
    return true;
}
/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/