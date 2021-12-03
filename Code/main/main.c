/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "../components/user_driver/flash/user_flash.h"
// #include "spiffs/src/spiffs.h"
#include "esp_spiffs.h"

#include "../Common.h"
#include "../task/user_console_interface.h"
#include "../task/plan_task.h"
#include "../task/audio_task.h"
// #include "../components/task/user_console_interface.h"
/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/

device_data_t device_data;

static void device_settup_default(void)
{
}

void app_main(void)
{
    APP_LOGI("--- APP_MAIN: Smart Toy Update 30/11/2021......");
    APP_LOGI("--- APP_MAIN: Free memory: %d bytes", esp_get_free_heap_size());
    device_settup_default();
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    flash_file_init();
    logger_list_file("/spiffs");
    check_map_size();

    plan_task();
    audio_task();
    console_task_start();
}
