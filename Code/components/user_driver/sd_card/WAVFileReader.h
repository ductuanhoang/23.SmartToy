#pragma once

#include "WAVFile.h"
#include <stdio.h>

void WAVFileReader_WAVFileReader(FILE *fp);
int WAVFileReader_sample_rate();
int WAVFileReader_read(int16_t *samples, int count);
