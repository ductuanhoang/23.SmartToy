/*
 * user_spi.c
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
#include "user_spi.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/
#define TOUCH_1_SPI_NUM HSPI_HOST
#define TOUCH_2_SPI_NUM VSPI_HOST

#define TOUCH_1_PIN_NUM_MISO 19
#define TOUCH_1_PIN_NUM_MOSI 23
#define TOUCH_1_PIN_NUM_CLK 18
#define TOUCH_1_PIN_NUM_CS 5

#define TOUCH_2_PIN_NUM_CS 17

#define GPIO_OUTPUT_PIN_CS (1ULL << TOUCH_1_PIN_NUM_CS) | (1ULL << TOUCH_2_PIN_NUM_CS)

/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/
#define EEPROM_CLK_FREQ (1 * 1000 * 1000) //When powered by 3.3V, EEPROM max freq is 1MHz
#define EEPROM_INPUT_DELAY_NS ((1000 * 1000 * 1000 / EEPROM_CLK_FREQ) / 2 + 20)

/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
static spi_device_handle_t touch1_spi_handle; // spi interface touch 1
static spi_device_handle_t touch2_spi_handle; // spi interface touch 2

static void spi_pre_transfer_callback(spi_transaction_t *t);
static void spi_post_transfer_callback(spi_transaction_t *t);

/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/
void user_spi_cs_pin_init(void)
{
    // init cs pin
}
/***********************************************************************************************************************
 * Function Name: user_spi_init
 * Description  : support initialization two different spi devices
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void user_spi_init(uint8_t spi_number)
{
    if (spi_number == 1)
    {
        /* Initial spi and parameters */
        APP_LOGD("spiInit: %d call\r\n", spi_number);
        esp_err_t ret;
        spi_bus_config_t buscfg = {
            .miso_io_num = TOUCH_1_PIN_NUM_MISO,
            .mosi_io_num = TOUCH_1_PIN_NUM_MOSI,
            .sclk_io_num = TOUCH_1_PIN_NUM_CLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
        };

        spi_device_interface_config_t devcfg_touch1 = {
            // .cs_ena_posttrans=3,//Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
            .clock_speed_hz = 1 * 100 * 1000,    // Clock out at 1 MHz
            .mode = 3,                           // SPI mode 3
            .spics_io_num = TOUCH_1_PIN_NUM_CS,  // CS pin set value for user control
            .queue_size = 20,                    // We want to be able to queue 7 transactions at a time
            .pre_cb = spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line
            .post_cb = spi_post_transfer_callback,
            .input_delay_ns = EEPROM_INPUT_DELAY_NS, // the EEPROM output the data half a SPI clock behind.
        };
        // spi device config for touch 2
        spi_device_interface_config_t devcfg_touch2 = {
            // .cs_ena_posttrans=3,//Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
            .clock_speed_hz = 1 * 100 * 1000,    // Clock out at 1 MHz
            .mode = 3,                           // SPI mode 3
            .spics_io_num = TOUCH_2_PIN_NUM_CS,  // CS pin set value for user control
            .queue_size = 20,                    // We want to be able to queue 7 transactions at a time
            .pre_cb = spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line
            .post_cb = spi_post_transfer_callback,
            .input_delay_ns = EEPROM_INPUT_DELAY_NS, // the EEPROM output the data half a SPI clock behind.
        };

        // Initialize the SPI bus
        ret = spi_bus_initialize(TOUCH_1_SPI_NUM, &buscfg, 0);
        assert(ret == ESP_OK);

        // Attach the touch to the SPI bus init for touch sensor 1
        ret = spi_bus_add_device(TOUCH_1_SPI_NUM, &devcfg_touch1, &touch1_spi_handle);
        assert(ret == ESP_OK);
        // config SPI handle for touch sensor 2
        ret = spi_bus_add_device(TOUCH_1_SPI_NUM, &devcfg_touch2, &touch2_spi_handle);
        assert(ret == ESP_OK);

        APP_LOGD("spiInit: %d ret = %d\r\n", spi_number, ret);
    }
}

/***********************************************************************************************************************
 * Function Name: spi_touch1_cs_pause
 * Description  : pause communication touch 1
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/

void spi_touch1_cs_pause(void)
{
    gpio_set_level(TOUCH_1_PIN_NUM_CS, 1);
}
/***********************************************************************************************************************
 * Function Name: spi_touch1_cs_resume
 * Description  : resume communication touch 1
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void spi_touch1_cs_resume(void)
{
    gpio_set_level(TOUCH_1_PIN_NUM_CS, 0);
}

/***********************************************************************************************************************
 * Function Name: spi_touch1_cs_pause
 * Description  : pause communication touch 1
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/

void spi_touch2_cs_pause(void)
{
    gpio_set_level(TOUCH_2_PIN_NUM_CS, 1);
}
/***********************************************************************************************************************
 * Function Name: spi_touch1_cs_resume
 * Description  : resume communication touch 1
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void spi_touch2_cs_resume(void)
{
    gpio_set_level(TOUCH_2_PIN_NUM_CS, 0);
}

/***********************************************************************************************************************
 * Function Name: spi_touch1_write
 * Description  : write the value to the device
 * Arguments    : uint8_t value
 * Return Value : none
 ***********************************************************************************************************************/
void spi_touch1_write(uint8_t value)
{
    esp_err_t ret;
    spi_transaction_t t;

    memset(&t, 0, sizeof(t)); // Zero out the transaction
    t.length = 8;             // Command is 8 bits
    t.tx_buffer = &value;     // The data is the cmd itself
    t.user = (void *)0;       // D/C needs to be set to 0
    ret = spi_device_transmit(touch1_spi_handle, &t);
    assert(ret == ESP_OK); // Should have had no issues.
    // return true;
}
/***********************************************************************************************************************
 * Function Name: spi_touch1_read
 * Description  : read the value from the device
 * Arguments    : uint8_t value
 * Return Value : none
 ***********************************************************************************************************************/

esp_err_t spi_touch1_read(uint8_t regAddr, uint8_t *data, uint8_t length)
{
    spi_transaction_t trans = {
        .length = (length)*8,
        .tx_buffer = &regAddr,
        .flags = SPI_TRANS_USE_RXDATA,
        .user = (void *)0,
    };
    esp_err_t err = spi_device_polling_transmit(touch1_spi_handle, &trans);
    if (err != ESP_OK)
    {
        printf("read data error\r\n");
        return err;
    }
    memcpy(data, trans.rx_data, 4);
    return err;
}
/***********************************************************************************************************************
 * Static Functions
 ***********************************************************************************************************************/
static void spi_pre_transfer_callback(spi_transaction_t *t)
{
    // APP_LOGD("spi_pre_transfer_callback\r\n");
}

static void spi_post_transfer_callback(spi_transaction_t *t)
{
    // APP_LOGD("call back transmit done\r\n");
}
/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/
