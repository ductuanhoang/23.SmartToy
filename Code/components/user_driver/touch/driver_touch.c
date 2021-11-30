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
#include "driver_touch.h"
#include "../../peripheral/user_spi.h"
/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/

typedef esp_err_t (*touch_read_fptr_t)(uint8_t regAddr, uint8_t *data, uint8_t length);
typedef void (*touch_write_fptr_t)(uint8_t value);
typedef void (*touch_cs)(void);
/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
typedef struct
{
    /*! touch number */
    uint8_t touch_number;
    /*! read function pointer */
    touch_read_fptr_t read;
    /*! write function pointer */
    touch_write_fptr_t write;
    /*! cs pin number */
    touch_cs cs_resume;
    touch_cs cs_pause;
    /*! Store the info messages */
    uint8_t info_msg;
} touch_struct_t;
/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
touch_struct_t touch_1;
touch_struct_t touch_2;
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name: driver_touch_init
 * Description  :
 * Arguments    : none
 * Return Value :
 ***********************************************************************************************************************/
void driver_touch_init(void)
{
    /*! Initial spi and parameters */
    user_spi_init(1); // initialize touch 1
    user_spi_init(2); // initialize touch 2
    /*! init call back functions for touch 1*/
    touch_1.read = spi_touch1_read;
    touch_1.write = spi_touch1_write;
    touch_1.cs_pause = spi_touch1_cs_pause;
    touch_1.cs_resume = spi_touch1_cs_resume;
    /*! init call back function for touch 2*/
    touch_2.read = spi_touch2_read;
    touch_2.write = spi_touch2_write;
    touch_2.cs_pause = spi_touch2_cs_pause;
    touch_2.cs_resume = spi_touch2_cs_resume;
    APP_LOGI(" Initial driver touch successfully");
}
/***********************************************************************************************************************
 * Function Name: driver_touch_process
 * Description  :
 * Arguments    : none
 * Return Value :
 ***********************************************************************************************************************/
void driver_touch_process(void)
{

}
/***********************************************************************************************************************
 * Static Functions
 ***********************************************************************************************************************/
typedef void (*read_callback)(uint8_t regAddr, uint8_t *data, uint8_t length);
bool AT42QT_GetID(uint8_t *id, read_callback read)
{
    bool status = false;
    uint8_t try_count = 0;
    uint8_t recv_data = 0x00;
    do
    {
        // delayMicroseconds(kDeviceCommDelay);
        read(kGetDeviceId, &recv_data, 1);
        if( (recv_data == kOK) | (recv_data == kGetDeviceId))
        {
            recv_data = 0x00;
            pause();
            // delayMicroseconds(kDeviceCommDelay);
            read(0x00, &recv_data, 1);
            if(recv_data == kDeviceId)
            {
                status = true;
                *id = recv_data;
            }
            else status = false;
        }
        else if(recv_data == kDeviceId)
        {
            status = true;
            *id = recv_data;
        }
        try_count++;
    }
    while((try_count < 30) & (status == false));
    return status;
}
/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/