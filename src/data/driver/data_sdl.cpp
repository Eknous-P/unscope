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

#include "data_sdl.h"
#include "audio_common.h"
#include <SDL_audio.h>
#include <data.h>

const int DataSDL::getFlags() {
  return DRIVERFLAG_OUTPUT|DRIVERFLAG_INPUT;
}

int DataSDL::setup(USCData* p) {
  parent = p;
  running = false;

  buffers = {};
  devices = {};

  deviceNum = 0;
  channels = 2;
  sampleRateNum = 0;
  frameSizeNum = 0;

  config = {
    Parameter(PARAM_INPUTINT, false, "channels", (void*)paramChannelsLimits, &channels),
    Parameter(PARAM_COMBO_INT, false, "sample rate", (void*)sampleRates, &sampleRateNum),
    Parameter(PARAM_COMBOV_STR, false, "device", &devices, &deviceNum),
    Parameter(PARAM_COMBO_INT, false, "frame size", (void*)frameSizes, &frameSizeNum),
  };

  if (SDL_Init(SDL_INIT_AUDIO)) return -1;
  return 0;
}

int DataSDL::init() {
  if (running) return -1;

  for (int i=0 ;i<buffers.size(); i++) {
    delete buffers[i];
  }
  buffers.clear();
  char strbuf[256];
  for (int i=0; i<channels; i++) {
    DataBuffer* newBuf = new DataBuffer_Float;
    snprintf(strbuf, 256, "SDL Input Channel %d", i+1);
    newBuf->init(65536, sampleRates[sampleRateNum], strbuf);
    buffers.push_back(newBuf);
  }

  request.userdata = this;
  request.callback = audioCallback;
  request.channels = channels;
  request.format = AUDIO_F32;
  request.freq = sampleRates[sampleRateNum];
  request.samples = frameSizes[frameSizeNum];

  deviceNumInternal = SDL_OpenAudioDevice(devices[deviceNum].c_str(), 1, &request, &response, 0);
  if (deviceNumInternal == 0) {
    printf(ERROR_MSG "OH NO!!! %s" MSG_END, SDL_GetError());
    return 1;
  }
  printf(INFO_MSG "SDL audio status: %d" MSG_END, SDL_GetAudioDeviceStatus(deviceNumInternal));
  return 0;
}

int DataSDL::enumerateDevices() {
  devices.clear();
  const int deviceNum = SDL_GetNumAudioDevices(1);
  printf(INFO_MSG "SDL detected %d drivers" MSG_END, SDL_GetNumAudioDrivers());
  for (int i=0; i<deviceNum; i++) {
    devices.push_back(SDL_GetAudioDeviceName(i, 1));
  }
  return deviceNum;
}

void DataSDL::audioCallback(void* userdata, Uint8* stream, int len) {
  DataSDL* self = (DataSDL*)userdata;
  float* samples = (float*)stream;
  const int samplesN = len/self->channels/sizeof(float);
  for (int i=0; i<samplesN; i++) {
    for (int j=0; j<self->channels; j++) {
      DataBuffer* buf = self->buffers[j];
      if(buf) buf->write(samples++);
    }
  }
}

int DataSDL::start() {
  SDL_PauseAudioDevice(deviceNumInternal, 0);
  return 0;
}

int DataSDL::stop() {
  SDL_CloseAudioDevice(deviceNumInternal);
  return 0;
}

int DataSDL::deinit() {
  SDL_CloseAudioDevice(deviceNumInternal);
  buffers.clear();
  return 0;
}

const char* DataSDL::getName() {
  return "SDL Driver";
}

