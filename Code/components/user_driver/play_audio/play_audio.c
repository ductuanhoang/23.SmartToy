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
#include "../../Common.h"
#include "play_audio.h"
#include "audio_files/wake_up.h"
#include "driver/i2s.h"
#include "driver/adc.h"

// include audio files
#include "audio_files/language1_page1_button1.h"
#include "audio_files/audio_wav_1.h"
/***********************************************************************************************************************
 * Macro definitions
 ***********************************************************************************************************************/
#define I2S_BCK_IO (27) // BCKL
#define I2S_WS_IO (26)  // LRCL
#define I2S_DO_IO (25)  // DOUT
#define I2S_DI_IO (-1)
#define EXAMPLE_I2S_NUM I2S_NUM_0

/***********************************************************************************************************************
 * Typedef definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables and functions
 ***********************************************************************************************************************/
static WavHeader_Struct WavHeader;
static unsigned const char *TheData;
static uint32_t DataIdx = 0; // index offset into "TheData" for current  data t send to I2S

static void i2s_init(void);
static void i2s_play_file(void);
static void play_selected_file(e_MODE_SELECTOR _mode, e_LANGUAGE_SELECTOR _language, e_PAGE_NUMBER _page_number, e_TOUCH_NUMBER _touch_number);
static void play_wakeup_file(void);

static void DumpWAVHeader(WavHeader_Struct *Wav);
static void PrintData(const char *Data, uint8_t NumBytes);

static bool ValidWavData(WavHeader_Struct *Wav);
/***********************************************************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Imported global variables and functions (from other files)
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Function Name: play_audio_init
 * Description  : initialize
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void play_audio_init(void)
{
    i2s_init();
}
/***********************************************************************************************************************
 * Function Name: play_audio_call_back
 * Description  :
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void play_audio_call_back(E_PLAY_AUDIO_TYPE _type,
                          e_MODE_SELECTOR _mode,
                          e_LANGUAGE_SELECTOR _language,
                          e_PAGE_NUMBER _page_number,
                          e_TOUCH_NUMBER _touch_number)
{
    APP_LOGD("play_audio_call_back called = %d", _type);
    if (_type == kWakeUpAudio) // random play audio form audio folder
    {
        play_wakeup_file();
    }
    else if (_type == kSelectedPlayAudio)
    {
        play_selected_file(_mode, _language, _page_number, _touch_number);
    }
}
/***********************************************************************************************************************
 * Static Functions
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * Function Name: play_wakeip_file
 * Description  : function play wake up audio folder
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
static void play_wakeup_file(void)
{
    i2s_play_file();
}
/***********************************************************************************************************************
 * Function Name: play_selected_file
 * Description  :
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
static void play_selected_file(e_MODE_SELECTOR _mode, e_LANGUAGE_SELECTOR _language, e_PAGE_NUMBER _page_number, e_TOUCH_NUMBER _touch_number)
{
    APP_LOGD("play audio mode: %d -- language: %d -- page: %d -- touch: %d", _mode, _language, _page_number, _touch_number);
    if (_mode == kMODE_TOP)
    {
        if (_language == kLANGUAGE_1)
        {
        }
        else if (_language == kLANGUAGE_2)
        {
        }
    }
    else if (_mode == kMODE_MIDDLE)
    {
        if (_language == kLANGUAGE_1)
        {
            /* code */
        }
        else if (_language == kLANGUAGE_2)
        {
        }
    }
    else if (_mode == kMODE_BOTTOM)
    {
        if (_language == kLANGUAGE_1)
        {
            /* code */
        }
        else if (_language == kLANGUAGE_2)
        {
        }
    }
}
/***********************************************************************************************************************
 * Function Name: i2s_init
 * Description  :
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
static void i2s_init(void)
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

    i2s_driver_install(EXAMPLE_I2S_NUM, &i2s_config, 0, NULL); // ESP32 will allocated resources to run I2S
    i2s_set_pin(EXAMPLE_I2S_NUM, &pin_config);                 // Tell it the pins you will be using
}
/***********************************************************************************************************************
 * Function Name: i2s_play_file
 * Description  :
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
static void i2s_play_file(void)
{
    APP_LOGI("Audio device play");
    memcpy(&WavHeader, &language1_page1_button1, 44); // Copy the header part of the wav data into our structure
    DumpWAVHeader(&WavHeader);
    if (ValidWavData(&WavHeader))
    {
        i2s_set_sample_rates(EXAMPLE_I2S_NUM, WavHeader.SampleRate); // set sample rate
        TheData = language1_page1_button1;                           // set to start of data
        TheData += 44;
    }
    while (1)
    {
        // 4. Play an example audio file(file format: 8bit/16khz/single channel)
        size_t BytesWritten; // Returned by the I2S write routine, we are not interested in it

        // As the WAV data for this example is in form of two 16 bit signed values we can send each four bytes direct to I2S
        i2s_write(EXAMPLE_I2S_NUM, TheData + DataIdx, 4, &BytesWritten, portMAX_DELAY);
        DataIdx += 4;                      // increase the data index to next two 16 bit values (4 bytes)
        if (DataIdx >= WavHeader.DataSize) // If we gone past end of data reset back to beginning
        {
            DataIdx = 0;
            return;
        }
    }
    TheData = 0;
    // memset(WavHeader, 0x00, sizeof(WavHeader));
}

static bool ValidWavData(WavHeader_Struct *Wav)
{
    if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
    {
        APP_LOGE("Invlaid data - Not RIFF format");
        return false;
    }
    if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
    {
        APP_LOGE("Invlaid data - Not Wave file");
        return false;
    }
    if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
    {
        APP_LOGE("Invlaid data - No format section found");
        return false;
    }
    if (memcmp(Wav->DataSectionID, "data", 4) != 0)
    {
        APP_LOGE("Invlaid data - data section not found");
        return false;
    }
    if (Wav->FormatID != 1)
    {
        APP_LOGE("Invlaid data - format Id must be 1");
        return false;
    }
    if (Wav->FormatSize != 16)
    {
        APP_LOGE("Invlaid data - format section size must be 16.");
        return false;
    }
    if ((Wav->NumChannels != 1) & (Wav->NumChannels != 2))
    {
        APP_LOGE("Invlaid data - only mono or stereo permitted.");
        return false;
    }
    if (Wav->SampleRate > 48000)
    {
        APP_LOGE("Invlaid data - Sample rate cannot be greater than 48000");
        return false;
    }
    if ((Wav->BitsPerSample != 8) & (Wav->BitsPerSample != 16))
    {
        APP_LOGE("Invlaid data - Only 8 or 16 bits per sample permitted.");
        return false;
    }
    return true;
}

/***********************************************************************************************************************
 * Function Name: DumpWAVHeader
 * Description  :
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
static void DumpWAVHeader(WavHeader_Struct *Wav)
{
    if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
    {
        APP_LOGE("Not a RIFF format file - ");
        PrintData(Wav->RIFFSectionID, 4);
        // return;
    }
    if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
    {
        APP_LOGE("Not a WAVE file - ");
        PrintData(Wav->RiffFormat, 4);
        // return;
    }
    if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
    {
        APP_LOGE("fmt ID not present - ");
        PrintData(Wav->FormatSectionID, 3);
        // return;
    }
    if (memcmp(Wav->DataSectionID, "data", 4) != 0)
    {
        APP_LOGE("data ID not present - ");
        PrintData(Wav->DataSectionID, 4);
        // return;
    }
    // All looks good, dump the data
    APP_LOGI("Total size : %d", Wav->Size);
    APP_LOGI("Format section size : %d", Wav->FormatSize);
    APP_LOGI("Wave format : %d", Wav->FormatID);
    APP_LOGI("Channels : %d", Wav->NumChannels);
    APP_LOGI("Sample Rate : %d", Wav->SampleRate);
    APP_LOGI("Byte Rate : %d", Wav->ByteRate);
    APP_LOGI("Block Align : %d", Wav->BlockAlign);
    APP_LOGI("Bits Per Sample : %d", Wav->BitsPerSample);
    APP_LOGI("Data Size : %d", Wav->DataSize);
}

static void PrintData(const char *Data, uint8_t NumBytes)
{
    for (uint8_t i = 0; i < NumBytes; i++)
        printf("%d ", Data[i]);
    printf("\r\n");
}

/***********************************************************************************************************************
 * End of file
 ***********************************************************************************************************************/