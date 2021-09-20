/*
 * page_selection.c
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
#include "page_selection.h"
#include "../../Common.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
#define GPIO_USER_PAGE_2 0
#define GPIO_USER_PAGE_3 0
#define GPIO_USER_PAGE_4 0
#define GPIO_USER_PAGE_5 0
#define GPIO_USER_PAGE_6 0
#define GPIO_USER_PAGE_7 0
#define GPIO_USER_PAGE_8 0

#define GPIO_USER_MODE_1 0
#define GPIO_USER_MODE_2 0
#define GPIO_USER_MODE_3 0

#define GPIO_USER_LANGUGE_1 0
#define GPIO_USER_LANGUGE_2 0
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static uint32_t time_page_selection = 0;
static uint8_t state_page_selection = 0;
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: e_PAGE_SELECTOR page_selection_detect
* Description  : get current page
* Arguments    : none
* Return Value : e_PAGE_SELECTOR
***********************************************************************************************************************/
e_PAGE_SELECTOR get_current_page_selection(void)
{
    e_PAGE_SELECTOR page_number = E_PAGE_1;
    if (gpio_get_level(GPIO_USER_PAGE_2) == 1)
    {
        page_number = E_PAGE_2;
    }
    else if (gpio_get_level(GPIO_USER_PAGE_3) == 1)
    {
        page_number = E_PAGE_3;
    }
    else if (gpio_get_level(GPIO_USER_PAGE_4) == 1)
    {
        page_number = E_PAGE_4;
    }
    else if (gpio_get_level(GPIO_USER_PAGE_5) == 1)
    {
        page_number = E_PAGE_5;
    }
    else if (gpio_get_level(GPIO_USER_PAGE_6) == 1)
    {
        page_number = E_PAGE_6;
    }
    else if (gpio_get_level(GPIO_USER_PAGE_7) == 1)
    {
        page_number = E_PAGE_7;
    }
    else if (gpio_get_level(GPIO_USER_PAGE_8) == 1)
    {
        page_number = E_PAGE_7;
    }
    else
        page_number = E_PAGE_1;
    return page_number;
}
/***********************************************************************************************************************
* Function Name: e_MODE_SELECTOR get_current_mode_selection(void)
* Description  : get current mode
* Arguments    : none
* Return Value : e_MODE_SELECTOR
***********************************************************************************************************************/
e_MODE_SELECTOR get_current_mode_selection(void)
{
    e_MODE_SELECTOR _mode = E_MODE_1;
    if (gpio_get_level(GPIO_USER_MODE_1) == 1)
    {
        _mode = E_MODE_1;
    }
    else if (gpio_get_level(GPIO_USER_MODE_2) == 1)
    {
        _mode = E_MODE_2;
    }
    else if (gpio_get_level(GPIO_USER_MODE_3) == 1)
    {
        _mode = E_MODE_3;
    }
    else
        _mode = E_MODE_1;

    return _mode;
}
/***********************************************************************************************************************
* Function Name: e_MODE_SELECTOR get_current_languge(void)
* Description  : get current language
* Arguments    : none
* Return Value : e_LANGUAGE_SELECTOR
***********************************************************************************************************************/
e_LANGUAGE_SELECTOR get_current_languge(void)
{
    e_LANGUAGE_SELECTOR _language = E_LANGUAGE_1;
    if (gpio_get_level(GPIO_USER_LANGUGE_1) == 1)
    {
        _language = E_LANGUAGE_1;
    }
    else if (gpio_get_level(GPIO_USER_LANGUGE_2) == 1)
    {
        _language = E_LANGUAGE_2;
    }
    return _language;
}
/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/

/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
