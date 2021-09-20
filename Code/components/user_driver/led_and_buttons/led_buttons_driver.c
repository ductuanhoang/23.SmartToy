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
#include "led_buttons_driver.h"
#include "../../Common.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
#define GPIO_USER_LED 0
#define GPIO_USER_BUTTON_1 0

#define TIME_LED_BLINK 100     // ms
#define TIME_BUTTONS_DETEC 300 //ms
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static uint8_t buttons_check(void);
static void led_blink(void);

static uint32_t time_led_previous = 0;
static uint8_t led_state = 0;
static uint8_t set_blink_time = 0;

/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
typedef void (*pHardButtonEventHandler)();

pHardButtonEventHandler _buttons_call_back = NULL;
/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void led_buttons_process(void)
{
    buttons_check();
    led_blink();
}

/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void led_set_blink(uint8_t _number_blink)
{
    set_blink_time = _number_blink;
}

void buttons_set_callback(pHardButtonEventHandler cb, void *data)
{
    if (cb != NULL)
        _buttons_call_back = cb;
    else APP_LOGE("error resgistor buttons");
}
/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void led_blink(void)
{
    if (set_blink_time > 0)
    {
        switch (led_state)
        {
        case 0:
            time_led_previous = usertimer_gettick();
            led_state = 1;
            gpio_set_level(GPIO_USER_LED, 1);
            /* code */
            break;
        case 1:
            if (usertimer_gettick() - time_led_previous > TIME_LED_BLINK)
            {
                gpio_set_level(GPIO_USER_LED, 0);
                led_state = 2;
                time_led_previous = usertimer_gettick();
            }
            break;
        case 2:
            if (usertimer_gettick() - time_led_previous > TIME_LED_BLINK)
            {
                led_state = 0;
                set_blink_time--;
            }
        default:
            break;
        }
    }
}
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static uint32_t time_buttons_previous = 0;
static uint8_t buttons_state = 0;
static uint8_t buttons_check(void)
{
    uint8_t _button_press = 0;
    switch (buttons_state)
    {
    case 0:
        time_buttons_previous = usertimer_gettick();
        if (gpio_get_level(GPIO_USER_BUTTON_1) == 1)
        {
            buttons_state = 1;
        }
        break;
    case 1:
        if (gpio_get_level(GPIO_USER_BUTTON_1) == 1)
        {
            if (usertimer_gettick() - time_buttons_previous > TIME_BUTTONS_DETEC)
            {
                // call back function
                _buttons_call_back();
                _button_press = 1;
                buttons_state = 0;
            }
        }
        else
            buttons_state = 0;
        break;

    default:
        break;
    }
    return _button_press;
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
