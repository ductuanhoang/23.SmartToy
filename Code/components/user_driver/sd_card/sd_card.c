/*
 * sd_card.c
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
#include "sd_card.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "WAVFileReader.h"
#include "play_audio/play_audio.h"

#include "esp_vfs.h"

/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
#define SD_CARD_SPI_NUM VSPI_HOST

#define SD_CARD_PIN_NUM_MISO 12 // 16 // 12
#define SD_CARD_PIN_NUM_MOSI 13
#define SD_CARD_PIN_NUM_CLK 14
#define SD_CARD_PIN_NUM_CS 15 // 17 // 15
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
static const char *TAG = "pdm_rec_example";

#define SPI_DMA_CHAN SPI_DMA_CH_AUTO
#define NUM_CHANNELS (1) // For mono recording only!
#define SD_MOUNT_POINT "/sdcard"
#define SAMPLE_SIZE (CONFIG_EXAMPLE_BIT_SAMPLE * 1024)
#define BYTE_RATE (CONFIG_EXAMPLE_SAMPLE_RATE * (CONFIG_EXAMPLE_BIT_SAMPLE / 8)) * NUM_CHANNELS
// Handle of the wear levelling library instance
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static void Output_write(int16_t *samples, int count);
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.
sdmmc_host_t host = SDSPI_HOST_DEFAULT();
sdmmc_card_t *card;
/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
/***********************************************************************************************************************
* Function Name: user_sd_card_init
* Description  : init spi of sd card
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void user_sd_card_init(void)
{
    // gpio_pull_up_en(GPIO_NUM_12);
    APP_LOGD("sd card mount");
    esp_err_t ret;
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 10 * 1024};
    ESP_LOGI(TAG, "Initializing SD card");

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_CARD_PIN_NUM_MOSI,
        .miso_io_num = SD_CARD_PIN_NUM_MISO,
        .sclk_io_num = SD_CARD_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    // reconfig host slot
    host.slot = SD_CARD_SPI_NUM;
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CARD_PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // user_sd_card_test();
    // DIR *dir = opendir("/sdcard/MODE1");
    // if (!dir)
    // {
    //     ESP_LOGE(TAG, "Failed to stat dir : %s", "/sdcard/MODE1/");
    //     /* Respond with 404 Not Found */
    // }
}

void user_sd_card_test(void)
{
    const char mount_point[] = SD_MOUNT_POINT;
    // Use POSIX and C standard library functions to work with files.

    // First create a file.
    const char *file_hello = SD_MOUNT_POINT "/hello.txt";

    ESP_LOGI(TAG, "Opening file %s", file_hello);
    FILE *f = fopen(file_hello, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    const char *file_foo = SD_MOUNT_POINT "/foo.txt";

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_foo, &st) == 0)
    {
        // Delete it if it exists
        unlink(file_foo);
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0)
    {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file %s", file_foo);
    f = fopen(file_foo, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    // Read a line from file
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos)
    {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(SD_CARD_SPI_NUM);
}

void user_play_audio(const char *fname)
{
    int16_t *samples = (int16_t *)malloc(sizeof(int16_t) * 1024);
    // open the file on the sdcard
    char file_name[50];
    sprintf(file_name, "%s/%s", SD_MOUNT_POINT, fname);
    ESP_LOGI(TAG, "Start open: %s", file_name);
    FILE *fp;
    // fp = fopen("/sdcard/MODE1/Langua1/1.wav", "rb");
    fp = fopen(file_name, "rb");
    if (fp == NULL)
    {
        ESP_LOGE(TAG, "open faild");
        return;
    }
    // create a new wave file writer
    ESP_LOGI(TAG, "create new wave file");
    WAVFileReader_WAVFileReader(fp);
    ESP_LOGI(TAG, "Start playing");

    // output->start(reader->sample_rate());
    i2s_set_sample_rates(EXAMPLE_I2S_NUM, WAVFileReader_sample_rate()); // set sample rate
    ESP_LOGI(TAG, "Opened wav file");
    // read until theres no more samples
    while (true)
    {
        int samples_read = WAVFileReader_read(samples, 1024);
        if (samples_read == 0)
        {
            break;
        }
        // ESP_LOGI(TAG, "Read %d samples", samples_read);
        Output_write(samples, samples_read);
        // ESP_LOGI(TAG, "Played samples");
    }
    fclose(fp);
}
/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/
// number of frames to try and send at once (a frame is a left and right sample)
const int NUM_FRAMES_TO_SEND = 256;
static void Output_write(int16_t *samples, int count)
{
    // this will contain the prepared samples for sending to the I2S device
    int16_t *frames = (int16_t *)malloc(2 * sizeof(int16_t) * NUM_FRAMES_TO_SEND);
    int sample_index = 0;
    while (sample_index < count)
    {
        int samples_to_send = 0;
        for (int i = 0; i < NUM_FRAMES_TO_SEND && sample_index < count; i++)
        {
            int sample = samples[sample_index];
            frames[i * 2] = sample;
            frames[i * 2 + 1] = sample;
            samples_to_send++;
            sample_index++;
        }
        // write data to the i2s peripheral
        size_t bytes_written = 0;
        i2s_write(EXAMPLE_I2S_NUM, frames, samples_to_send * sizeof(int16_t) * 2, &bytes_written, portMAX_DELAY);
        if (bytes_written != samples_to_send * sizeof(int16_t) * 2)
        {
            ESP_LOGE(TAG, "Did not write all bytes");
        }
    }
    free(frames);
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
