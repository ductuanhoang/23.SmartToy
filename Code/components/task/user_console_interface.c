/*
 * user_console_interface.c
 *
 *  Created on: Nov 24, 2020
 *      Author: Yolo
 */
/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "user_console_interface.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"

#include "cmd_nvs/cmd_nvs.h"
#include "cmd_system/cmd_system.h"

/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
static const char *TAG = "example";
#define PROMPT_STR CONFIG_IDF_TARGET
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static void console_task(void *pvParameters);
static void initialize_console(void);

/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void console_task_start(void)
{
	xTaskCreate(console_task, "console_task", 3 * 1024, NULL, 2 | portPRIVILEGE_BIT, NULL);
}

static void console_task(void *pvParameters)
{
	initialize_console();
	/* Register commands */
	esp_console_register_help_command();
	register_system();
	register_nvs();
	/* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */

	const char *prompt = LOG_COLOR_I PROMPT_STR "> " LOG_RESET_COLOR;

	printf("\n Command user start\n");

	/* Figure out if the terminal supports escape sequences */
	int probe_status = linenoiseProbe();
	if (probe_status)
	{ /* zero indicates success */
		printf("\n"
			   "Your terminal application does not support escape sequences.\n"
			   "Line editing and history features are disabled.\n"
			   "On Windows, try using Putty instead.\n");
		linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
		/* Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
		prompt = PROMPT_STR "> ";
#endif //CONFIG_LOG_COLORS
	}

	/* Main loop */
	while (true)
	{
		/* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
		char *line = linenoise(prompt);
		if (line == NULL)
		{ /* Break on EOF or error */
			// break;
			ESP_LOGE(TAG, "Error no command detection console");
		}
		else
		{

			/* Add the command to the history if not empty*/
			if (strlen(line) > 0)
			{
				linenoiseHistoryAdd(line);
#if CONFIG_STORE_HISTORY
				/* Save command history to filesystem */
				linenoiseHistorySave(HISTORY_PATH);
#endif
			}
			/* Try to run the command */
			int ret;
			esp_err_t err = esp_console_run(line, &ret);
			if (err == ESP_ERR_NOT_FOUND)
			{
				printf("Unrecognized command\n");
			}
			else if (err == ESP_ERR_INVALID_ARG)
			{
				// command was empty
			}
			else if (err == ESP_OK && ret != ESP_OK)
			{
				printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
			}
			else if (err != ESP_OK)
			{
				printf("Internal error: %s\n", esp_err_to_name(err));
			}
			/* linenoise allocates line buffer on the heap, so need to free it */
			linenoiseFree(line);
		}
	}
}
/***********************************************************************************************************************
* static functions
***********************************************************************************************************************/
static void initialize_console(void)
{
	/* Drain stdout before reconfiguring it */
	fflush(stdout);
	fsync(fileno(stdout));

	/* Disable buffering on stdin */
	setvbuf(stdin, NULL, _IONBF, 0);

	/* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
	esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR);
	/* Move the caret to the beginning of the next line on '\n' */
	esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF);

	/* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
	const uart_config_t uart_config = {
		.baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.source_clk = UART_SCLK_REF_TICK,
	};
	/* Install UART driver for interrupt-driven reads and writes */
	ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM,
										256, 0, 0, NULL, 0));
	ESP_ERROR_CHECK(uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));

	/* Tell VFS to use UART driver */
	esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

	/* Initialize the console */
	esp_console_config_t console_config = {.max_cmdline_args = 8,
										   .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
										   .hint_color = atoi(LOG_COLOR_CYAN)
#endif
	};
	ESP_ERROR_CHECK(esp_console_init(&console_config));

	/* Configure linenoise line completion library */
	/* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
	linenoiseSetMultiLine(1);

	/* Tell linenoise where to get command completions and hints */
	linenoiseSetCompletionCallback(&esp_console_get_completion);
	linenoiseSetHintsCallback((linenoiseHintsCallback *)&esp_console_get_hint);

	/* Set command history size */
	linenoiseHistorySetMaxLen(100);

	/* Don't return empty lines */
	linenoiseAllowEmpty(false);

#if CONFIG_STORE_HISTORY
	/* Load command history from filesystem */
	linenoiseHistoryLoad(HISTORY_PATH);
#endif
}

/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
