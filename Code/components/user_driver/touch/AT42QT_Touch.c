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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../peripheral/user_spi.h"
#include "AT42QT_Touch.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/
#define NOP() asm volatile("nop")
/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
static void AT42QT_1_write(uint8_t value);
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

static DeviceStatus_t AT42QT_GetStatus(void);
static bool AT42QT_GetCmd(uint8_t cmd, uint8_t *data, uint8_t length);
static bool AT42QT_SetCmd(uint8_t addr, uint8_t data);
static void AT42QT_Save(void);
static void AT42QT_Reset(void);
static void AT42QT_Configure(uint8_t numkeys, uint8_t guardkey);
static void AT42QT_Calibrate(void);
static uint8_t AT42QT_GetKey();
static uint16_t AT42QT_GetAllKeys(void);
static void AT42QT_Erase(void);
static bool AT42QT_GetID(uint8_t *id);
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

touch_buttons_handler event_callback_handler;
/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

void AT42QT_1_init(void)
{
    user_spi_init(1);
    AT42QT_Configure(11, 9);
    AT42QT_Calibrate();
    uint8_t id;
    if (AT42QT_GetID(&id))
        APP_LOGD("id device = %x\r\n", id);
    else
        APP_LOGD("id device error\r\n");
    
}

void AT42QT_1_process(void)
{
    DeviceStatus_t user_get_status = AT42QT_GetStatus();
    if (user_get_status.detect == 1)
    {
        uint8_t key_number = AT42QT_GetKey();
        APP_LOGD("key_number = %d\r\n", key_number);
        event_callback_handler(1, key_number, NULL);
    }
}

void AT42QT_1_set_callback(touch_buttons_handler cb)
{
    event_callback_handler = cb;
}
/***********************************************************************************************************************
 * Static Functions AT42QT1
 ***********************************************************************************************************************/
static void AT42QT_1_write(uint8_t value)
{
    spi_touch1_write(value);
}

static void AT42QT_1_read(uint8_t regAddr, uint8_t *data, uint8_t length)
{
    spi_touch1_read(regAddr, data, length);
}

static void AT42QT_1_cs_pause(void)
{
    spi_touch1_cs_pause();
}

static void AT42QT_1_cs_resume(void)
{
    spi_touch1_cs_resume();
}

/***********************************************************************************************************************
 * Function Name: AT42QT_GetID
 * Description  :
 * Arguments    : none
 * Return Value :
 ***********************************************************************************************************************/
static bool AT42QT_GetID(uint8_t *id)
{
    bool status = false;
    uint8_t try_count = 0;
    uint8_t recv_data = 0x00;
    do
    {
        delayMicroseconds(kDeviceCommDelay);
        AT42QT_1_read(kGetDeviceId, &recv_data, 1);
        if ((recv_data == kOK) | (recv_data == kGetDeviceId))
        {
            recv_data = 0x00;
            AT42QT_1_cs_pause();
            delayMicroseconds(kDeviceCommDelay);
            AT42QT_1_read(0x00, &recv_data, 1);
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

static bool AT42QT_GetVer(uint8_t *ver)
{
    bool status = false;
    uint8_t try_count = 0;
    uint8_t recv_data = 0x00;
    do
    {
        AT42QT_1_read(kGetFirmwareVer, &recv_data, 1);
        if (recv_data == kOK)
        {
            recv_data = 0x00;
            AT42QT_1_cs_pause();
            delayMicroseconds(kDeviceCommDelay);
            AT42QT_1_read(0x00, &recv_data, 1);
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

static DeviceStatus_t AT42QT_GetStatus(void)
{
    union
    {
        uint8_t c;
        DeviceStatus_t stat;
    } ret;
    // vTaskDelay(300/portTICK_RATE_MS);
    // delayMicroseconds(kDeviceCommDelay);
    bool status = AT42QT_GetCmd(kDeviceStatusCmd, &ret.c, 1);
    delayMicroseconds(kDeviceCommDelay);
    // APP_LOGD("data read = %x\r\n", ret.c);
    // APP_LOGD("status = %d\r\n", status);
    return ret.stat;
}
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
    AT42QT_1_write(kSaveToEeprom);
    vTaskDelay(150 / portTICK_RATE_MS);
}

static void AT42QT_Reset(void)
{
    AT42QT_1_write(kChipReset);
    vTaskDelay(150 / portTICK_RATE_MS); // after reset device need 150ms
}

static void AT42QT_Calibrate(void)
{
    delayMicroseconds(kDeviceCommDelay);
    AT42QT_1_write(kCalibrateAll);
    vTaskDelay(150 / portTICK_RATE_MS); // after reset device need 150ms
}

static void AT42QT_Erase(void)
{
    AT42QT_1_write(kEraseEeprom);
    vTaskDelay(150 / portTICK_RATE_MS); // after reset device need 150ms
}
static bool AT42QT_GetCmd(uint8_t cmd, uint8_t *data, uint8_t length)
{
    bool status = false;
    uint8_t try_count = 0;
    do
    {
        AT42QT_1_read(cmd, data, 1);
        if ((data[0] == kOK) | (data[0] == cmd))
        {
            AT42QT_1_cs_pause();
            delayMicroseconds(kDeviceCommDelay);
            AT42QT_1_read(0x00, data, length);
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
    AT42QT_1_write(kWriteCmd + addr);
    delayMicroseconds(kDeviceCommDelay);
    AT42QT_1_write(data);
    return true;
}
/***********************************************************************************************************************
 * Static Functions AT42QT2
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/
