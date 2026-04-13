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


const DataDriverInfo DataSDL::getDriverInfo() const {
  return {
    DATA_SDL,
    DRIVERFLAG_OUTPUT|DRIVERFLAG_INPUT|DRIVERFLAG_PAUSE,
    "SDL Driver"
  };
}

int DataSDL::setup(USCData* p) {
  parent = p;
  state = 0;

  buffers = {};
  devices = {};

  deviceNum = 0;
  channels = 2;
  sampleRateNum = 6;
  frameSizeNum = 5;

  config = {
    Parameter(PARAM_INPUTINT, false, "channels", "channels", NULL, (void*)paramChannelsLimits, &channels),
    Parameter(PARAM_COMBO_INT, false, "sampleRate", "sample rate", NULL, (void*)sampleRates, &sampleRateNum),
    Parameter(PARAM_COMBOV_STR, false, "device", "device",  NULL,&devices, &deviceNum),
    Parameter(PARAM_COMBO_INT, false, "frameSize", "frame size", NULL, (void*)frameSizes, &frameSizeNum),
  };

  for (int i=0; i<16; i++) {
    buffers.push_back(new DataBuffer_Float);
  }

  if (SDL_Init(SDL_INIT_AUDIO)) return -1;

  state |= DRIVERSTATE_READY|DRIVERSTATE_OK;

  return 0;
}

void DataSDL::activate() {
  if (!(state&DRIVERSTATE_ACTIVE))  {
    doPlay(false);
    deactivate();
    if (state&DRIVERSTATE_ERROR) {
      printf(ERROR_MSG "SDL: failed to deactivate device!" MSG_END);
      return;
    }
  }
  if (!(state&(DRIVERSTATE_OK|DRIVERSTATE_READY)))  {
    printf(ERROR_MSG "SDL: cannot activate!" MSG_END);
    state |= DRIVERSTATE_ERROR;
    return;
  }
  for (int i=0 ;i<buffers.size(); i++) {
    delete buffers[i];
  }
  buffers.clear();
  char strbuf[256];
  for (int i=0; i<16; i++)
    buffers[i]->destroy();
  for (int i=0; i<channels; i++) {
    snprintf(strbuf, 256, "SDL Input Channel %d", i+1);
    buffers[i]->init(65536, sampleRates[sampleRateNum+1], strbuf);
  }

  request.userdata = this;
  request.callback = audioCallback;
  request.channels = channels;
  request.format = AUDIO_F32;
  request.freq = sampleRates[sampleRateNum+1];
  request.samples = frameSizes[frameSizeNum+1];

  deviceNumInternal = SDL_OpenAudioDevice(devices[deviceNum].c_str(), 1, &request, &response, 0);
  if (deviceNumInternal == 0) {
    printf(ERROR_MSG "SDL: OH NO!!! %s" MSG_END, SDL_GetError());
    state |= DRIVERSTATE_ERROR;
    return;
  }
  printf(INFO_MSG "SDL audio status: %d" MSG_END, SDL_GetAudioDeviceStatus(deviceNumInternal));
  state |= DRIVERSTATE_ACTIVE;
  return;
}

void DataSDL::doPlay(bool play) {
  if (!(state&DRIVERSTATE_ACTIVE)) {
    // state |= DRIVERSTATE_ERROR;
    return;
  }
  if (play) {
    if (state&DRIVERSTATE_PLAY) return;
    SDL_PauseAudioDevice(deviceNumInternal, 0);
    state |= DRIVERSTATE_PLAY;
  } else {
    if (!(state&DRIVERSTATE_PLAY)) return;
    SDL_PauseAudioDevice(deviceNumInternal, 1);
    state &=~DRIVERSTATE_PLAY;
  }
}

int DataSDL::enumerateDevices() {
  devices.clear();
  const int deviceNum = SDL_GetNumAudioDevices(1);
  printf(INFO_MSG "SDL detected %d drivers" MSG_END, SDL_GetNumAudioDrivers());
  for (int i=0; i<deviceNum; i++) {
    devices.push_back(SDL_GetAudioDeviceName(i, 1));
  }
  return 0;
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

void DataSDL::deactivate() {
  if (!(state&(DRIVERSTATE_OK|DRIVERSTATE_READY))) return;
  if (state&DRIVERSTATE_PLAY) doPlay(false);
  SDL_CloseAudioDevice(deviceNumInternal);
  state&=~DRIVERSTATE_ACTIVE;
}

void DataSDL::destroy() {
  if (state&DRIVERSTATE_ACTIVE) deactivate();
  for (int i=0; i<config.size(); i++) config[i].destroy();
  config.clear();
  for (int i=0; i<buffers.size(); i++) buffers[i]->destroy();
  buffers.clear();
}

DataSDL::~DataSDL() {
  devices.clear();
}
