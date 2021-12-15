#include "esp_log.h"
#include "WAVFileReader.h"

static const char *TAG = "WAV";

static wav_header_t m_wav_header =
    {
        .wav_size = 0,
        .fmt_chunk_size = 16,
        .audio_format = 1,
        .num_channels = 1,
        .sample_rate = 16000,
        .byte_rate = 32000,
        .sample_alignment = 2,
        .bit_depth = 16,
        .data_bytes = 0,
        .riff_header[0] = 'R',
        .riff_header[1] = 'I',
        .riff_header[2] = 'F',
        .riff_header[3] = 'F',
        .wave_header[0] = 'W',
        .wave_header[1] = 'A',
        .wave_header[2] = 'V',
        .wave_header[3] = 'E',
        .fmt_header[0] = 'f',
        .fmt_header[1] = 'm',
        .fmt_header[2] = 't',
        .fmt_header[3] = ' ',
        .data_header[0] = 'd',
        .data_header[1] = 'a',
        .data_header[2] = 't',
        .data_header[3] = 'a',
};

static FILE *m_fp;

void WAVFileReader_WAVFileReader(FILE *fp)
{
    // setup define value of struct
    m_fp = fp;
    // read the WAV header
    ESP_LOGI(TAG, "read wav header");
    fread((void *)&m_wav_header, sizeof(wav_header_t), 1, m_fp);
    ESP_LOGI(TAG, "pass read wav header");
    // sanity check the bit depth
    if (m_wav_header.bit_depth != 16)
    {
        ESP_LOGE(TAG, "ERROR: bit depth %d is not supported\n", m_wav_header.bit_depth);
    }
    if (m_wav_header.num_channels != 1)
    {
        ESP_LOGE(TAG, "ERROR: channels %d is not supported\n", m_wav_header.num_channels);
    }
    ESP_LOGI(TAG, "fmt_chunk_size=%d, audio_format=%d, num_channels=%d, sample_rate=%d, sample_alignment=%d, bit_depth=%d, data_bytes=%d\n",
             m_wav_header.fmt_chunk_size, m_wav_header.audio_format, m_wav_header.num_channels, m_wav_header.sample_rate, m_wav_header.sample_alignment, m_wav_header.bit_depth, m_wav_header.data_bytes);
}

int WAVFileReader_read(int16_t *samples, int count)
{
    size_t read = fread(samples, sizeof(int16_t), count, m_fp);
    return read;
}

int WAVFileReader_sample_rate() { return m_wav_header.sample_rate; }