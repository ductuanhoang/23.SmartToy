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


#define NOP() asm volatile ("nop")

typedef struct
{
    uint8_t guard:1;
    uint8_t reset:1;
    uint8_t eeprom:1;
    uint8_t change:1;

    uint8_t error:1;
    uint8_t cycle:1;
    uint8_t detect:1;
    uint8_t one:1;   
} DeviceStatus_t;

static bool AT42QT_GetID(uint8_t *id);
static bool AT42QT_GetVer(uint8_t *ver);
static DeviceStatus_t AT42QT_GetStatus(void);
static bool AT42QT_GetCmd(uint8_t cmd, uint8_t* data, uint8_t length);
static bool AT42QT_SetCmd(uint8_t addr, uint8_t data);
static void AT42QT_Save(void);
static void AT42QT_Reset(void);
static void AT42QT_Configure(uint8_t numkeys, uint8_t guardkey);
static void AT42QT_Calibrate(void);
static uint8_t AT42QT_GetKey();
static uint16_t AT42QT_GetAllKeys(void);
static void AT42QT_Erase(void);

unsigned long IRAM_ATTR micros()
{
    return (unsigned long) (esp_timer_get_time());
}
void IRAM_ATTR delayMicroseconds(uint32_t us)
{
    uint32_t m = micros();
    if(us){
        uint32_t e = (m + us);
        if(m > e){ //overflow
            while(micros() > e){
                NOP();
            }
        }
        while(micros() < e){
            NOP();
        }
    }
}

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
        // .max_transfer_sz = 32
    };

    spi_device_interface_config_t devcfg = {
        // .cs_ena_posttrans=3,//Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
        .clock_speed_hz = 1 * 100 * 1000,       //Clock out at 1 MHz
        .mode = 3,                              //SPI mode 3
        .spics_io_num = PIN_NUM_CS,             //CS pin
        .queue_size = 20,                        //We want to be able to queue 7 transactions at a time
        .pre_cb = spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
        .post_cb = spi_post_transfer_callback,
        .input_delay_ns = EEPROM_INPUT_DELAY_NS,  //the EEPROM output the data half a SPI clock behind.
    };

    //Initialize the SPI bus
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0);
    assert(ret == ESP_OK);

    //Attach the touch to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
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

esp_err_t spiReadVal(uint8_t regAddr, uint8_t* data, uint8_t length)
{
    spi_transaction_t trans = {
        .length = (length )* 8,
        // .cmd = 8,
        .tx_buffer = &regAddr,
        .flags = SPI_TRANS_USE_RXDATA,
        .user = (void*)0,
        // .rx_buffer = data,
    };
    esp_err_t err = spi_device_polling_transmit(spi, &trans);
    if (err!= ESP_OK)
    {
        printf("read data error\r\n");
        return err;
    }
    // printf("data 1 = %x\r\n", data[0]);
    // printf("data 2 = %x\r\n", trans.rx_buffer[0]);
    // printf("data 3 = %x\r\n", trans.rx_buffer[2]);
    // printf("data 4 = %x\r\n", trans.rx_buffer[3]);
    // else *data = trans.rx_data[0];
    memcpy(data, trans.rx_data, 4);
    return err;
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

void pause(void)
{
    gpio_set_level(PIN_NUM_CS, 1);
}

void resume(void)
{
     gpio_set_level(PIN_NUM_CS, 0);
}

void app_main(void)
{
    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
    // user_gpio_init();
    uint8_t id;
    uint8_t ver;
    user_spi_init();
    // vTaskDelay(200/ portTICK_RATE_MS);
    // AT42QT_Erase();
    // vTaskDelay(200/ portTICK_RATE_MS);
    // if(AT42QT_GetID(&id))
    //     APP_LOGD("id device = %x\r\n", id);
    // else APP_LOGD("id device error\r\n");
    // vTaskDelay(1000 / portTICK_RATE_MS);
    // bool status = false;
    AT42QT_Configure(11, 9);
    AT42QT_Calibrate();
    vTaskDelay(250 / portTICK_RATE_MS);
    while(1) {
        DeviceStatus_t user_get_status = AT42QT_GetStatus();
        // // APP_LOGD("status 1 = 0x%2x\r\n", user_get_status.guard);
        // // APP_LOGD("status 2 = 0x%2x\r\n", user_get_status.reset);
        // // APP_LOGD("status 3 = 0x%2x\r\n", user_get_status.eeprom);
        // // APP_LOGD("status 4 = 0x%2x\r\n", user_get_status.change);
        // // APP_LOGD("status 5 = 0x%2x\r\n", user_get_status.error);
        // // APP_LOGD("status 6 = 0x%2x\r\n", user_get_status.cycle);
        // // APP_LOGD("status 7 = 0x%2x\r\n", user_get_status.detect);
        // // APP_LOGD("status 8 = 0x%2x\r\n", user_get_status.one);
        if( user_get_status.detect == 1)
        {
            uint8_t key_number = AT42QT_GetKey();
            APP_LOGD("key_number = %d\r\n", key_number);
        }
        // if(AT42QT_GetID(&id))
        //     APP_LOGD("id device = %x\r\n", id);
        // else APP_LOGD("id device error\r\n");
        vTaskDelay(100 / portTICK_RATE_MS);
    }
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
* Static Functions
// SPI user lib
***********************************************************************************************************************/

enum QT1110Misc
{
    kOK = 0x55,
    kDeviceId = 0x57,
    kDeviceCommDelay = 150
};

enum QT1110Commands
{
// Control
    kSendSetups = 0x1,
    kCalibrateAll = 0x3,
    kChipReset = 0x4,
    kSleep = 0x5,
    kSaveToEeprom = 0xA,
    kLoadFromEeprom = 0xB,
    kEraseEeprom    = 0xc,
    kRecoverEeprom  = 0xd,
    kCalibrateKeyN   = 0x10,

    // Report
    kSendFirstKey = 0xC0,
    kSendAllKeys  = 0xC1,
    kDeviceStatusCmd = 0xC2,
    kEepromCrc    = 0xC3,
    kRamCrc       = 0xC4,
    kErrorKeys    = 0xC5,
    kSignalKeyN   = 0x20,
    kRefKeyN      = 0x40,
    kStatusKeyN   = 0x80,
    kWriteCmd     = 0x90,
    kDetectOutputStates = 0xC6,
    kLastCommand  = 0xC7,
    kGetSetups    = 0xC8,
    kGetDeviceId     = 0xC9,
    kGetFirmwareVer  = 0xCA
};


enum QT1110HardCodedSetup
{
    kKEY_AC      = 1,
    kSIGNAL      = 1,
    kSYNC        = 1,
    kREPEAT_TIME = 2,//8,

    kQUICK_SPI   = 0,
    kCHG         = 1,
    kCRC         = 0
};

bool AT42QT_GetID(uint8_t *id)
{
    bool status = false;
    uint8_t try_count = 0;
    uint8_t recv_data = 0x00;
    do
    {
        delayMicroseconds(kDeviceCommDelay);
        spiReadVal(kGetDeviceId, &recv_data, 1);
        if( (recv_data == kOK) | (recv_data == kGetDeviceId))
        {
            recv_data = 0x00;
            pause();
            delayMicroseconds(kDeviceCommDelay);
            spiReadVal(0x00, &recv_data, 1);
            if(recv_data == kDeviceId)
            {
                status = true;
                *id = recv_data;
            }
            else status = false;
        }
        else if(recv_data == kDeviceId)
        {
            status = true;
            *id = recv_data;
        }
        try_count++;
    }
    while((try_count < 30) & (status == false));
    return status;
}


bool AT42QT_GetVer(uint8_t *ver)
{
    bool status = false;
    uint8_t try_count = 0;
    uint8_t recv_data = 0x00;
    do
    {
        spiReadVal(kGetFirmwareVer, &recv_data, 1);
        if( recv_data == kOK)
        {
            recv_data = 0x00;
            pause();
            delayMicroseconds(kDeviceCommDelay);
            spiReadVal(0x00, &recv_data, 1);
            if((recv_data != kOK) && (recv_data != kGetFirmwareVer))
            {
                status = true;
                *ver = recv_data;
            }
            else status = false;
        }
        try_count++;
    }
    while((try_count < 30) & (status == false));
    return status;
}

DeviceStatus_t AT42QT_GetStatus(void)
{
    union
    {
        uint8_t c;
        DeviceStatus_t stat;
    } ret;
    // vTaskDelay(300/portTICK_RATE_MS);
    // delayMicroseconds(kDeviceCommDelay);
    bool status = AT42QT_GetCmd(kDeviceStatusCmd, &ret.c, 1);
    delayMicroseconds(kDeviceCommDelay);
    // APP_LOGD("data read = %x\r\n", ret.c);
    // APP_LOGD("status = %d\r\n", status);
    return ret.stat;
}
static uint8_t AT42QT_GetKey()
{
    uint8_t recv_data;
    AT42QT_GetCmd(kSendFirstKey, &recv_data, 1);
    if ((recv_data&0x80)==0) return 0;
    return (recv_data&0xf)+1;
}

static uint16_t AT42QT_GetAllKeys(void)
{
  uint16_t ret;
  delayMicroseconds(kDeviceCommDelay); // In multibyte communications, the master must pause for a minimum delay of 150 us between the completion of one byte exchange and the beginning of the next.
  AT42QT_GetCmd(kSendAllKeys, &ret, 2);
  return (ret<<1);  // Shift the bitmap by one bit to 
}

static void AT42QT_Configure(uint8_t numkeys, uint8_t guardkey)
{
  if (numkeys==11) numkeys=1;
  else numkeys=0;
  
  uint8_t data =  (kKEY_AC<<7) | (numkeys<<6) | (kSIGNAL<<5) | (kSYNC<<4) | (kREPEAT_TIME&0xf);
  AT42QT_SetCmd(0, data);

  if (guardkey>0) data =  (((guardkey-1)&0xf)<<4) | (1<<3);  // set the guard key number and GD_EN
  else data = 0; 
  data |= (kQUICK_SPI<<2) | (kCHG<<1) | (kCRC<<0);
  AT42QT_SetCmd(1, data);

  // Atmel firmware bug requires a reset (or maybe a load()) to actually switch the mode
  AT42QT_Save();
  AT42QT_Reset();
}

static void AT42QT_Save(void)
{
    delayMicroseconds(kDeviceCommDelay);
    spiWriteVal(kSaveToEeprom);
    vTaskDelay(150/ portTICK_RATE_MS);  
}

static void AT42QT_Reset(void)
{
    spiWriteVal(kChipReset);
    vTaskDelay(150/ portTICK_RATE_MS); // after reset device need 150ms
}

static void AT42QT_Calibrate(void)
{
    delayMicroseconds(kDeviceCommDelay);
    spiWriteVal(kCalibrateAll);
    vTaskDelay(150/ portTICK_RATE_MS); // after reset device need 150ms
}

static void AT42QT_Erase(void)
{
    spiWriteVal(kEraseEeprom);
    vTaskDelay(150/ portTICK_RATE_MS); // after reset device need 150ms
}
static bool AT42QT_GetCmd(uint8_t cmd, uint8_t* data, uint8_t length)
{
    bool status = false;
    uint8_t try_count = 0;
    do
    {
        spiReadVal(cmd, data, 1);
        if( (data[0] == kOK) | (data[0] == cmd))
        {
            pause();
            delayMicroseconds(kDeviceCommDelay);
            spiReadVal(0x00, data, length);
            if((data[0] != kOK) && (data[0] != cmd))
            {
                status = true;
            }
            else status = false;
        }
        try_count++;
    }
    while((try_count < 30) & (status == false));
    return status;
}


static bool AT42QT_SetCmd(uint8_t addr, uint8_t data)
{
    if (addr>41) 
    {
        return false;
    }
    delayMicroseconds(kDeviceCommDelay);
    spiWriteVal(kWriteCmd + addr);
    delayMicroseconds(kDeviceCommDelay);
    spiWriteVal(data);
    return true;
}