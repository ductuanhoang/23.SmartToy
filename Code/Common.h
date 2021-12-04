/*
 * Common.h
 *
 *  Created on: Apr 24, 2021
 *      Author: ductu
 */

#ifndef MAIN_COMMON_H_
#define MAIN_COMMON_H_
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "main.h"

enum
{
	E_LOG_LVL_NONE,
	E_LOG_LVL_INFO,
	E_LOG_LVL_ERROR,
	E_LOG_LVL_WARNING,
	E_LOG_LVL_DEBUG,
	E_LOG_LVL_NEVER
};

/* Console color */
#define RESET "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"
/*Background colors*/
#define BG_KOLORS_BLK "\x1b[40m" // Black
#define BG_KOLORS_RED "\x1b[41m" // Red
#define BG_KOLORS_GRN "\x1b[42m" // Green
#define BG_KOLORS_YEL "\x1b[43m" // Yellow
#define BG_KOLORS_BLU "\x1b[44m" // Blue

#define LOG_SHOULD_I(level) (level <= LOG_BUILD_LEVEL && level <= E_LOG_LVL_DEBUG)
#define LOG(level, tag, ...)                                       \
	do                                                             \
	{                                                              \
		if (LOG_SHOULD_I(level))                                   \
		{                                                          \
			printf("[%s] %s:%d: " RESET, tag, __func__, __LINE__); \
			printf(__VA_ARGS__);                                   \
			printf("\r\n");                                        \
		}                                                          \
	} while (0)

#define APP_LOGE(...) LOG(E_LOG_LVL_ERROR, KRED "ERROR" RESET, __VA_ARGS__)
#define APP_LOGI(...) LOG(E_LOG_LVL_INFO, KGRN "INFOR" RESET, __VA_ARGS__)
#define APP_LOGD(...) LOG(E_LOG_LVL_DEBUG, KYEL "DEBUG" RESET, __VA_ARGS__)
#define APP_LOGW(...) LOG(E_LOG_LVL_WARNING, BG_KOLORS_YEL "ALARM" RESET, __VA_ARGS__)

#define LOG_BUILD_LEVEL E_LOG_LVL_DEBUG

/**
 * @brief define button input
 *
 */
#define GPIO_USER_BUTTON 0
#define GPIO_INPUT_BUTTONS_PIN_SEL (1ULL << GPIO_USER_BUTTON)

#define GPIO_USER_LED 2 // led k
#define GPIO_OUTPUT_PIN_SEL (1ULL << GPIO_USER_LED)

/**
 * @brief define GPIO mode, page and languge
 *
 */

#define GPIO_USER_MODE_2 36
#define GPIO_USER_MODE_3 39

#define GPIO_USER_LANGUGE_1 22

#define GPIO_USER_PAGE_2 16
#define GPIO_USER_PAGE_3 4
#define GPIO_USER_PAGE_4 21
#define GPIO_USER_PAGE_5 33
#define GPIO_USER_PAGE_6 32
#define GPIO_USER_PAGE_7 35
#define GPIO_USER_PAGE_8 34

#define GPIO_INPUT_LANGUAGE_PIN_SEL (1ULL << GPIO_USER_MODE_2) | (1ULL << GPIO_USER_MODE_3) | (1ULL << GPIO_USER_LANGUGE_1) | (1ULL << GPIO_USER_PAGE_2) | (1ULL << GPIO_USER_PAGE_3) | (1ULL << GPIO_USER_PAGE_4) | (1ULL << GPIO_USER_PAGE_5) | (1ULL << GPIO_USER_PAGE_6) | (1ULL << GPIO_USER_PAGE_7) | (1ULL << GPIO_USER_PAGE_8)

typedef enum
{
	kMODE_TOP = 0,
	kMODE_MIDDLE,
	kMODE_BOTTOM
} e_MODE_SELECTOR;

typedef enum
{
	kLANGUAGE_1,
	kLANGUAGE_2
} e_LANGUAGE_SELECTOR;

typedef enum
{
	kPageLeft,
	kPageRight
} e_PAGE_LEFT_RIGHT;

typedef enum
{
	/*
	 * when open the book we will see 2 pages in the book the
	 * the left side is page 1 and the right side is page 2 (for example)
	 */
	kBOTH_PAGE_CLOSE = 0,
	kBOTH_PAGE_1_2 = 1,
	kBOTH_PAGE_3_4 = 2,
	kBOTH_PAGE_5_6 = 4,
	kBOTH_PAGE_7_8 = 6,
	kBOTH_PAGE_9_10 = 8,
	kBOTH_PAGE_11_12 = 10,
} e_BOTH_PAGE_SELECTOR;

typedef enum
{
	kTOUCH_IDLE = 0,
	kTOUCH_0 = 1,
	kTOUCH_1,
	kTOUCH_2,
	kTOUCH_3,
	kTOUCH_4,
	kTOUCH_5,
	kTOUCH_6,
	kTOUCH_7,
	kTOUCH_8,
	kTOUCH_9,
} e_TOUCH_NUMBER;

typedef enum
{
	kPAGE_1 = 0,
	kPAGE_2,
	kPAGE_3,
	kPAGE_4,
	kPAGE_5,
	kPAGE_6,
	kPAGE_7,
	kPAGE_8,
	kPAGE_9,
	kPAGE_10,
	kPAGE_11,
	kPAGE_12,
} e_PAGE_NUMBER;

typedef enum
{
	kDEVICE_OFF = 0,
	kDEVICE_ON,
} e_DEVICE_STATE;

typedef struct
{
	e_MODE_SELECTOR mode;
	e_LANGUAGE_SELECTOR languge;
	e_BOTH_PAGE_SELECTOR both_page;
	e_TOUCH_NUMBER touch_number;
	e_PAGE_NUMBER page_number;
} device_data_t;

extern device_data_t device_data;

extern uint32_t usertimer_gettick(void);
#endif /* MAIN_COMMON_H_ */
