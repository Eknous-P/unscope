#ifndef EXTDATA_H
#define EXTDATA_H

#include "shared.h"

static constexpr int channelsClamp[2] = {
  1,
  2
};

static constexpr int sampleRateSelectableData[9] = {
  8000,
  11025,
  16000,
  22050,
  32000,
  44100,
  48000,
  96000,
  128000,
};

static constexpr int bufferSizeClamp[1] = {
  512
};

static constexpr int frameSizeSelectableData[7] = {
  32,
  64,
  128,
  256,
  512,
  1024,
  2048
};

#endif
