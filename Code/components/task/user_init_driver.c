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
#include "../user_driver/play_audio/play_audio.h"
#include "../user_driver/touch/AT42QT_Touch.h"
#include "../peripheral/user_timer.h"

#include "../user_driver/sd_card/sd_card.h"
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
typedef void (*events_handler_play_audio)(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
static void touch_event_handler(uint8_t touch_key, e_PAGE_LEFT_RIGHT touch_page);
static void vsm_btn_event_press(int btn_idx, int event, void *p);
static void user_buttons_setup(void);

static void touch_event_press(uint8_t btn_idx, uint8_t event, void *p);
static void touch_2_event_press(uint8_t btn_idx, uint8_t event, void *p);
static uint8_t touch_change_right_to_user(uint8_t btn);
static uint8_t touch_change_left_to_user(uint8_t btn);

static tsButtonConfig btnParams[] = BOARD_BTN_CONFIG;
static events_handler_play_audio user_play_audio_call;
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/
extern void audio_task_activate_callback(E_PLAY_AUDIO_TYPE _type,
                                         e_MODE_SELECTOR _mode,
                                         e_LANGUAGE_SELECTOR _language,
                                         e_PAGE_NUMBER _page_number,
                                         e_TOUCH_NUMBER _touch_number);
/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/
void init_callback(events_handler_play_audio cb)
{
    if (cb == NULL)
    {
        APP_LOGE("pls initialize cb function");
    }
    else
    {
        user_play_audio_call = cb;
    }
}
/***********************************************************************************************************************
 * Function Name: user_driver_init
 * Description  : initialize peripheral
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void user_driver_init(void)
{
    // initialize Touch device
    AT42QT_1_init();
    AT42QT_2_init();
    // initialize timer
    user_timer_init();
    // initialize audio device
    play_audio_init();
    // initialize led driver
    user_leds_init(); // initialize gpio
    // init buttons gpio
    buttons_gpio_init();
    // call back function for button
    user_buttons_setup();
    // init both_page, language, mode gpio
    language_gpio_init();

    // callback event touch and buttons handler
    init_callback(audio_task_activate_callback);

    // set call back touch buttons
    AT42QT_1_set_callback(touch_event_press);
    AT42QT_2_set_callback(touch_2_event_press);
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
    // process touch event
    AT42QT_1_process();
    AT42QT_2_process();
}
/***********************************************************************************************************************
 * Static Functions
 ***********************************************************************************************************************/
void user_driver_check_param(void)
{
    APP_LOGD("languge = %d", device_data.languge);
    APP_LOGD("mode = %d", device_data.mode);
    APP_LOGD("both_page = %d", device_data.both_page);

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
static void touch_event_handler(uint8_t touch_key, e_PAGE_LEFT_RIGHT touch_page)
{
    device_data.page_number = device_data.both_page + touch_page;
    device_data.touch_number = touch_key;
    // send touch event to play audio of the device
    if (device_data.both_page != kBOTH_PAGE_CLOSE)
        user_play_audio_call(kSelectedPlayAudio, device_data.mode, device_data.languge, device_data.page_number, device_data.touch_number);
}

/***********************************************************************************************************************
 * Function Name: user_buttons_setup
 * Description  : setup user callback function for buttons event handler
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
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
        // callbacks function play audio  wake up
        user_play_audio_call(kWakeUpAudio, 0, 0, 0, 0);
        user_leds_call(kLED_BLINK, 0xFF);
        // if (device_data.both_page != kBOTH_PAGE_CLOSE)
        // {
        //     user_play_audio_call(kWakeUpAudio, 0, 0, 0, 0);
        //     user_leds_call(kLED_BLINK, 0xFF);
        // }
        break;
    default:
        break;
    }
}
static uint8_t active_touch_buttons = 0;

static void touch_event_press(uint8_t btn_idx, uint8_t event, void *p)
{
    uint8_t user_button = 0;
    // APP_LOGD("AT1 active touch %d pressed", btn_idx);
    if ((active_touch_buttons == 0) && (btn_idx != 0))
    {
        active_touch_buttons = 1;
    }
    if (btn_idx == 0)
    {
        active_touch_buttons = 0;
    }
    else
    {
        if (active_touch_buttons == 1)
        {
            active_touch_buttons = 2;
            // change to user button in requirement
            user_button = touch_change_right_to_user(btn_idx);
            touch_event_handler(user_button, kPageRight);
        }
    }
}

static void touch_2_event_press(uint8_t btn_idx, uint8_t event, void *p)
{
    uint8_t user_button = 0;
    // APP_LOGD("AT2 active touch %d pressed", btn_idx);
    if ((active_touch_buttons == 0) && (btn_idx != 0))
    {
        active_touch_buttons = 1;
    }
    if (btn_idx == 0)
    {
        active_touch_buttons = 0;
    }
    else
    {
        if (active_touch_buttons == 1)
        {
            active_touch_buttons = 2;
            // change to user button in requirement
            user_button = touch_change_left_to_user(btn_idx);
            touch_event_handler(user_button, kPageLeft);
        }
    }
}
// some diffents between user requirments and schematic so need to change to user button
static uint8_t touch_change_right_to_user(uint8_t btn)
{
    uint8_t user_button = 1;
    switch (btn)
    {
    case 1:
        user_button = 8;
        /* code */
        break;
    case 2:
        user_button = 2;
        break;
    case 3:
        user_button = 9;
        break;
    case 4:
        user_button = 5;
        break;
    case 5:
        user_button = 3;
        break;
    case 6:
        user_button = 4;
        break;
    case 7:
        user_button = 6;
        break;
    case 8:
        user_button = 7;
        break;
    case 9:
        break;
    case 10:
        user_button = 1;
        break;
    default:
        break;
    }
    return user_button;
}

// some diffents between user requirments and schematic so need to change to user button
static uint8_t touch_change_left_to_user(uint8_t btn)
{
    uint8_t user_button = 1;
    switch (btn)
    {
    case 1:
        user_button = 2;
        /* code */
        break;
    case 2:
        user_button = 9;
        break;
    case 3:
        user_button = 5;
        break;
    case 4:
        user_button = 3;
        break;
    case 5:
        user_button = 4;
        break;
    case 6:
        user_button = 6;
        break;
    case 7:
        user_button = 7;
        break;
    case 8:
        break;
    case 9:
        break;
    case 10:
        user_button = 8;
        break;
    default:
        break;
    }
    return user_button;
}
/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/
