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
#define BOARD_BTN_CONFIG                                            \
    {                                                               \
        /*Last state   Idle level    Btn Type   pin   Callback */   \
        {0, 1, 1, GPIO_USER_BUTTON}, /* Rotary Encoder Channel A */ \
    }
/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
static void touch_event_handler(uint8_t touch_key, uint8_t touch_page);
static void (*events_handler_play_audio)(uint8_t, uint8_t, uint8_t, uint8_t);
static void vsm_btn_event_press(int btn_idx, int event, void *p);
static void user_buttons_setup(void);

static tsButtonConfig btnParams[] = BOARD_BTN_CONFIG;
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
    // call back function for button
    user_buttons_setup();
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
    buttons_process(NULL);
    device_data.languge = get_current_languge();
    device_data.mode = get_current_mode_selection();
    device_data.both_page = get_current_page_selection();
}
/***********************************************************************************************************************
 * Static Functions
 ***********************************************************************************************************************/
void user_driver_check_param(void)
{
    APP_LOGD("languge = %d", device_data.languge);
    APP_LOGD("mode = %d", device_data.mode);
    APP_LOGD("both_page = %d", device_data.both_page);
    user_leds_call(kLED_BLINK, 5);
    page_get_level_pin();
    get_mode_test_gpio();
}
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

static void user_buttons_setup(void)
{
    vHardButtonSetGetTickCallback(usertimer_gettick);
    vHardButtonInit(btnParams, 1);
    vHardButtonSetCallback(E_EVENT_HARD_BUTTON_PRESS, vsm_btn_event_press, NULL);
}

/***********************************************************************************************************************
* Function Name: vsm_btn_event_release
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void vsm_btn_event_press(int btn_idx, int event, void *p)
{
    switch (btn_idx)
    {
    case 0:
        APP_LOGD("vsm_btn_event_press = %d", btn_idx);
        break;
    default:
        break;
    }
}
/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/
