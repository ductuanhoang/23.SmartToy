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
#include "user_leds.h"
/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
static void led_blinking_process(void);
static void led_state_active(void);
static void user_gpio_set_level(bool _status);


static user_led_status_t user_led_status;
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name: user_leds_init
 * Description  : init gpio leds
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void user_leds_init(void)
{
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    esp_err_t error = gpio_config(&io_conf); // configure GPIO with the given settings
    if (error != ESP_OK)
    {
        APP_LOGE("error configuring outputs\n");
    }

    // setup value user_led_status
    user_led_status.number_blinking = 5;
    user_led_status.time_blinking = 500; // default value is 500ms interval
    user_led_status.type = kLED_OFF;
}
/***********************************************************************************************************************
 * Function Name: user_leds_call
 * Description  : setup user LED status
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void user_leds_call(e_LED_STATUS_TYPE type, uint8_t number_blinking)
{
    if (number_blinking != 0)
        user_led_status.number_blinking = number_blinking;
    else
        user_led_status.number_blinking = 5;

    user_led_status.type = type;

    // active led processing
    if (type == kLED_BLINK)
        led_state_active();
}
/***********************************************************************************************************************
 * Function Name: user_leds_process
 * Description  : call in loop to process the LED status
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void user_leds_process(void)
{
    if (user_led_status.type == kLED_OFF)
    {
        user_gpio_set_level(0);
    }
    else if (user_led_status.type == kLED_ON)
    {
        user_gpio_set_level(1);
    }
    else if (user_led_status.type == kLED_BLINK)
    {
        led_blinking_process();
    }
}
/***********************************************************************************************************************
 * Static Functions
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * Function Name: user_leds_process
 * Description  : call in loop to process the LED status
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
static int8_t leds_state = -1;
static uint32_t timming_state = 0;
static uint8_t leds_count = 0;
static void led_blinking_process(void)
{
    switch (leds_state)
    {
    case -1: // idle state
        break;
    case 0:
        timming_state = usertimer_gettick();
        user_gpio_set_level(1); // led off
        leds_state = 1;
        break;
    case 1:
        if (leds_count == 0)
            timming_state = 3;
        else if (usertimer_gettick() - timming_state > user_led_status.time_blinking)
        {
            user_gpio_set_level(0); // led on
            timming_state = 2;
            timming_state = usertimer_gettick();
            leds_count--;
        }
        break;
    case 2:
        if (usertimer_gettick() - timming_state > user_led_status.time_blinking)
        {
            timming_state = 0;
        }
        break;
    case 3:
        leds_count = user_led_status.number_blinking;
        leds_state = -1;
        break;
    default:
        break;
    }
}
/***********************************************************************************************************************
 * Function Name: led_state_active
 * Description  : active led state
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
static void led_state_active(void)
{
    leds_state = 0;
    timming_state = 0;
    leds_count = user_led_status.number_blinking;
}
/***********************************************************************************************************************
 * Function Name: gpio_set_level
 * Description  : set led level
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
static void user_gpio_set_level(bool _status)
{
    gpio_set_level(GPIO_USER_LED, _status);
}
/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/
