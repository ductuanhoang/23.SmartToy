#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spi_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "driver/i2s.h"
#include "driver/adc.h"
// #include "audio_example_file.h"
#include "audio_wav_1.h"
#include "esp_adc_cal.h"
// #include "esp_rom_sys.h"

#define PARTITION_NAME "storage"

/*---------------------------------------------------------------
                            EXAMPLE CONFIG
---------------------------------------------------------------*/

#define I2S_BCK_IO (27) // BCKL
#define I2S_WS_IO (26)  // LRCL
#define I2S_DO_IO (25)  // DOUT
#define I2S_DI_IO (-1)
#define EXAMPLE_I2S_NUM I2S_NUM_0

typedef struct
{
    //   RIFF Section
    char RIFFSectionID[4]; // Letters "RIFF"
    uint32_t Size;         // Size of entire file less 8
    char RiffFormat[4];    // Letters "WAVE"

    //   Format Section
    char FormatSectionID[4]; // letters "fmt"
    uint32_t FormatSize;     // Size of format section less 8
    uint16_t FormatID;       // 1=uncompressed PCM
    uint16_t NumChannels;    // 1=mono,2=stereo
    uint32_t SampleRate;     // 44100, 16000, 8000 etc.
    uint32_t ByteRate;       // =SampleRate * Channels * (BitsPerSample/8)
    uint16_t BlockAlign;     // =Channels * (BitsPerSample/8)
    uint16_t BitsPerSample;  // 8,16,24 or 32

    // Data Section
    char DataSectionID[4]; // The letters "data"
    uint32_t DataSize;     // Size of the data that follows
} WavHeader_Struct;

static void PrintData(const char *Data, uint8_t NumBytes);
static void DumpWAVHeader(WavHeader_Struct *Wav);
static bool ValidWavData(WavHeader_Struct *Wav);
static void example_i2s(void *arg);

unsigned const char *TheData;
uint32_t DataIdx = 0; // index offset into "TheData" for current  data t send to I2S



WavHeader_Struct WavHeader;

esp_err_t app_main(void)
{
    // example_i2s_init();
    esp_log_level_set("I2S", ESP_LOG_INFO);
    xTaskCreate(example_i2s, "example_i2s", 1024 * 2, NULL, 5, NULL);
    return ESP_OK;
}

/*
 * Function run the I2S driver
 */
static void example_i2s(void *arg)
{
    // 1. init i2s interface
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_DO_IO,
        .data_in_num = I2S_DI_IO // Not used
    };

    int i2s_num = EXAMPLE_I2S_NUM;

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100, // Note, this will be changed later
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
        .dma_buf_count = 8,                       // 8 buffers
        .dma_buf_len = 1024,                      // 1K per buffer, so 8K of buffer space
        .use_apll = 0,
        .tx_desc_auto_clear = true,
        .fixed_mclk = -1,
    };

    memcpy(&WavHeader, &audio_table, 44); // Copy the header part of the wav data into our structure
    DumpWAVHeader(&WavHeader);
    if (ValidWavData(&WavHeader))
    {
        i2s_driver_install(EXAMPLE_I2S_NUM, &i2s_config, 0, NULL);   // ESP32 will allocated resources to run I2S
        i2s_set_pin(EXAMPLE_I2S_NUM, &pin_config);                   // Tell it the pins you will be using
        i2s_set_sample_rates(EXAMPLE_I2S_NUM, WavHeader.SampleRate); // set sample rate
        TheData = audio_table;                                           // set to start of data
        TheData += 44;
    }

    while (1)
    {
        // 4. Play an example audio file(file format: 8bit/16khz/single channel)
        size_t BytesWritten; // Returned by the I2S write routine, we are not interested in it

        // As the WAV data for this example is in form of two 16 bit signed values we can send each four bytes direct to I2S
        i2s_write(i2s_num, TheData + DataIdx, 4, &BytesWritten, portMAX_DELAY);
        DataIdx += 4;                      // increase the data index to next two 16 bit values (4 bytes)
        if (DataIdx >= WavHeader.DataSize) // If we gone past end of data reset back to beginning
            DataIdx = 0;
    }
    vTaskDelete(NULL);
}

static bool ValidWavData(WavHeader_Struct *Wav)
{
    if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
    {
        printf("Invlaid data - Not RIFF format");
        return false;
    }
    if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
    {
        printf("Invlaid data - Not Wave file");
        return false;
    }
    if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
    {
        printf("Invlaid data - No format section found");
        return false;
    }
    if (memcmp(Wav->DataSectionID, "data", 4) != 0)
    {
        printf("Invlaid data - data section not found");
        return false;
    }
    if (Wav->FormatID != 1)
    {
        printf("Invlaid data - format Id must be 1");
        return false;
    }
    if (Wav->FormatSize != 16)
    {
        printf("Invlaid data - format section size must be 16.");
        return false;
    }
    if ((Wav->NumChannels != 1) & (Wav->NumChannels != 2))
    {
        printf("Invlaid data - only mono or stereo permitted.");
        return false;
    }
    if (Wav->SampleRate > 48000)
    {
        printf("Invlaid data - Sample rate cannot be greater than 48000");
        return false;
    }
    if ((Wav->BitsPerSample != 8) & (Wav->BitsPerSample != 16))
    {
        printf("Invlaid data - Only 8 or 16 bits per sample permitted.");
        return false;
    }
    return true;
}

static void DumpWAVHeader(WavHeader_Struct *Wav)
{
    if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
    {
        printf("Not a RIFF format file - ");
        PrintData(Wav->RIFFSectionID, 4);
        // return;
    }
    if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
    {
        printf("Not a WAVE file - ");
        PrintData(Wav->RiffFormat, 4);
        // return;
    }
    if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
    {
        printf("fmt ID not present - ");
        PrintData(Wav->FormatSectionID, 3);
        // return;
    }
    if (memcmp(Wav->DataSectionID, "data", 4) != 0)
    {
        printf("data ID not present - ");
        PrintData(Wav->DataSectionID, 4);
        // return;
    }
    // All looks good, dump the data
    printf("Total size : %d", Wav->Size);
    printf("Format section size : %d", Wav->FormatSize);
    printf("Wave format : %d", Wav->FormatID);
    printf("Channels : %d", Wav->NumChannels);
    printf("Sample Rate : %d", Wav->SampleRate);
    printf("Byte Rate : %d", Wav->ByteRate);
    printf("Block Align : %d", Wav->BlockAlign);
    printf("Bits Per Sample : %d", Wav->BitsPerSample);
    printf("Data Size : %d", Wav->DataSize);
}

static void PrintData(const char *Data, uint8_t NumBytes)
{
    for (uint8_t i = 0; i < NumBytes; i++)
        printf("%d ", Data[i]);
    printf("\r\n");
}
