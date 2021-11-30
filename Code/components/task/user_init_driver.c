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
#include "user_init_driver.h"
// Include driver
#include "../user_driver/buttons/user_buttons.h"
#include "../user_driver/leds/user_leds.h"
#include "../user_driver/page_selection/page_selection.h"
#include "../user_driver/page_selection/language_selection.h"
#include "../user_driver/page_selection/mode_selection.h"

/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
static void touch_event_handler(uint8_t touch_key, uint8_t touch_page);
static void (*events_handler_play_audio)(uint8_t, uint8_t, uint8_t, uint8_t);
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name: user_driver_init
 * Description  : initialize peripheral
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void user_driver_init(void)
{
    // initialize led driver
    user_leds_init(); // initialize gpio
    // init buttons gpio
    buttons_gpio_init();
    // init both_page, language, mode gpio
    language_gpio_init();
}

/***********************************************************************************************************************
 * Function Name: user_driver_process
 * Description  : call in loop to process peripheral
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void user_driver_process(void)
{
    user_leds_process(); // leds process
    device_data.languge = get_current_languge();
    device_data.mode = get_current_mode_selection();
    device_data.both_page = get_current_page_selection();
}
/***********************************************************************************************************************
 * Static Functions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name:
 * Description  :
 * Arguments    : touch_key: key number to be pressed
 *                touch_page: both_page number to be selected
 * Return Value : none
 ***********************************************************************************************************************/
static void touch_event_handler(uint8_t touch_key, uint8_t touch_page)
{
    device_data.page_number = device_data.both_page + touch_page;
    device_data.touch_number = touch_key;
    // send touch event to play audio of the device
    events_handler_play_audio(device_data.mode, device_data.languge, device_data.page_number, device_data.touch_number);
}
/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/
