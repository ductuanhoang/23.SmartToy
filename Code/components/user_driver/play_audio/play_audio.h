/*
 * play_audio.h
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */

#ifndef MAIN_USER_DRIVER_PLAY_AUDIO
#define MAIN_USER_DRIVER_PLAY_AUDIO

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "../../Common.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/*!
 * @brief enum for type definitions play audio
 * @details
 *
 */
typedef enum
{
    kWakeUpAudio,
    kSelectedPlayAudio,
} E_PLAY_AUDIO_TYPE;
/*!
 * @brief struct for type definitions play audio files
 * @details
 *
 */
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
/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/

/***********************************************************************************************************************
 * Function Name:
 * Description  :
 * Arguments    : none
 * Return Value : none
 ***********************************************************************************************************************/
void play_audio_init(void);

void play_audio_call_back(E_PLAY_AUDIO_TYPE _type,
                          e_MODE_SELECTOR _mode,
                          e_LANGUAGE_SELECTOR _language,
                          e_PAGE_NUMBER _page_number,
                          e_TOUCH_NUMBER _touch_number);

#endif /* MAIN_USER_DRIVER_PLAY_AUDIO */
