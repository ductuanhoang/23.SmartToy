/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS 5
#define GPIO_OUTPUT_PIN_CS (1ULL << PIN_NUM_CS)
//To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
//but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
#define PARALLEL_LINES 16

#define APP_LOGD printf

static void spi_pre_transfer_callback(spi_transaction_t *t);
static void spi_post_transfer_callback(spi_transaction_t *t);

static spi_device_handle_t spi;

#define EEPROM_CLK_FREQ         (1*1000*1000)   //When powered by 3.3V, EEPROM max freq is 1MHz
#define EEPROM_INPUT_DELAY_NS   ((1000*1000*1000/EEPROM_CLK_FREQ)/2+20)

void user_spi_init(void)
{
    APP_LOGD("spiInit call\r\n");
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,       //Clock out at 1 MHz
        .mode = 3,                              //SPI mode 3
        .spics_io_num = PIN_NUM_CS,             //CS pin
        .queue_size = 20,                        //We want to be able to queue 7 transactions at a time
        .pre_cb = spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
        .post_cb = spi_post_transfer_callback,
        .input_delay_ns = EEPROM_INPUT_DELAY_NS,  //the EEPROM output the data half a SPI clock behind.
    };

    //Initialize the SPI bus
    ret = spi_bus_initialize(VSPI_HOST, &buscfg, 0);
    assert(ret == ESP_OK);

    //Attach the touch to the SPI bus
    ret = spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
    assert(ret == ESP_OK);

    APP_LOGD("spiInit oke ret = %d\r\n", ret);
}

uint8_t spiWriteVal(uint8_t value)
{
    esp_err_t ret;
    spi_transaction_t t;

    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = 8;                   //Command is 8 bits
    t.tx_buffer = &value;             //The data is the cmd itself
    t.user = (void*)0;              //D/C needs to be set to 0
//    ret = spi_device_polling_transmit(spi, &t); //Transmit!
    ret = spi_device_transmit(spi, &t);
    assert(ret == ESP_OK);          //Should have had no issues.

    return true;
}

esp_err_t spiReadVal(uint8_t addr, uint16_t* out_data)
{
    spi_transaction_t t = {
        .length = 16,
        // .cmd = addr,
        .tx_buffer = &addr,
        .rxlength = 16,
        .flags = SPI_TRANS_USE_RXDATA,
        .user = (void*)0,
    };
    esp_err_t err = spi_device_polling_transmit(spi, &t);
    if (err!= ESP_OK) return err;

    *out_data = t.rx_data[0];
    return ESP_OK;
}

void user_gpio_init(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;

    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_CS;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    esp_err_t error = gpio_config(&io_conf);

    if (error != ESP_OK)
    {
        APP_LOGD("error configuring inputs\n");
    }
}

void app_main(void)
{
    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
    user_gpio_init();

    uint16_t out_data;
    uint16_t version;
    user_spi_init();

    while(1) {
        spiReadVal(0xC9, &out_data);
        APP_LOGD("out_data = %x\r\n", out_data);
        out_data = 0x00;
        spiReadVal(0xCA, &version);
        APP_LOGD("version raw = %x\r\n", version);
        APP_LOGD("version = %d.%d\r\n", version&0xF0 >> 4, version & 0x0F);
        version = 0;
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}


/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/
static void spi_pre_transfer_callback(spi_transaction_t *t)
{
    APP_LOGD("spi_pre_transfer_callback");
}

static void spi_post_transfer_callback(spi_transaction_t *t)
{
    APP_LOGD("call back transmit done");
}
