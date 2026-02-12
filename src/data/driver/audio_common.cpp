/*
unscope - an audio oscilloscope
Copyright (C) 2025-2026 Eknous

unscope is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

unscope is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
unscope. If not, see <https://www.gnu.org/licenses/>. 
*/

#include "audio_common.h"

const int sampleRates[10]={
  9,
  8000,
  11025,
  16000,
  22050,
  32000,
  44100,
  48000,
  96000,
  192000
};

const int frameSizes[9]={
  8,
  16, 32, 64, 128,
  256, 512, 1024, 2048
};

const int paramChannelsLimits[2] = {1, 16};
