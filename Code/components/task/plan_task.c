/*
 * led_driver.c
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
#include "../../Common.h"
#include "plan_task.h"
#include "user_init_driver.h"
// Include driver

/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
static void PlantControl_Task(void *pvParameters);
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name: 
 * Description  : initialize peripheral
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name: 
 * Description  : call in loop to process peripheral
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void plan_task(void)
{
    xTaskCreatePinnedToCore(PlantControl_Task, "plant_task", 6 * 1024, NULL, 2 | portPRIVILEGE_BIT, NULL, 1);
}

/***********************************************************************************************************************
 * Static Functions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name:
 * Description  :
 * Arguments    : 
 * Return Value : none
 ***********************************************************************************************************************/
static void PlantControl_Task(void *pvParameters)
{
    uint32_t count = 0;
    user_driver_init();
    while (1)
    {
        user_driver_process();
        if (count > 1000)
        {
            count = 0;
            // user_driver_check_param();
        }
        count++;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/
