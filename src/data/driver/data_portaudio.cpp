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

#include "data_portaudio.h"
#include "portaudio.h"
#include "audio_common.h"

const DataDriverInfo DataPortAudio::getDriverInfo() const {
  return {
    DATA_PORTAUDIO,
    DRIVERFLAG_OUTPUT|DRIVERFLAG_INPUT|DRIVERFLAG_PAUSE,
    "PortAudio Driver"
  };
}

int DataPortAudio::setup(USCData* p) {
  parent = p;
  state = 0;
  paInitSuccess = false;

  buffers = {};
  inputDevices = {};
  outputDevices = {};
  outputting = false;

  inputChannels = outputChannels = 0;

  inputChannelsP = 2;
  outputChannelsP = 0;

  config = {
    Parameter(PARAM_INPUTINT, false, "inputChans", "input channels", NULL, (void*)paramChannelsLimits, &inputChannelsP),
    // Parameter(PARAM_INPUTINT, false, "outputChans", "output channels", NULL, (void*)paramChannelsLimits, &outputChannelsP),
    Parameter(PARAM_COMBO_INT, false, "sampleRate", "sample rate", NULL, (void*)sampleRates, &sampleRateP),
    Parameter(PARAM_COMBOV_STR, false, "inputDevice", "input device", NULL, &inputDevices, &inputDeviceP),
    // Parameter(PARAM_COMBOV_STR, false, "outputDevice", "output device", NULL, &outputDevices, &outputDeviceP),
    Parameter(PARAM_COMBO_INT, false, "frameSize", "frame size", NULL, (void*)frameSizes, &frameSizeP),
  };

  sampleRateP = 6;
  inputDeviceP = outputDeviceP = 0;
  frameSizeP = 5;

  for (int i=0; i<16; i++) {
    buffers.push_back(new DataBuffer_Float);
  }

  printf(INFO_MSG "PA: initializing PortAudio..." MSG_END);
  e = Pa_Initialize();
  if (e!=paNoError) {
    printf(ERROR_MSG "PA: failed to initialize PortAudio! %s" MSG_END, Pa_GetErrorText(e));
    return e;
  }
  else paInitSuccess = true;

  stream=NULL;

  if (paInitSuccess) {
    printf(SUCCESS_MSG "PA: initialized successfully" MSG_END);
    state |=  DRIVERSTATE_OK|DRIVERSTATE_READY;
    return 0;
  }
  return 1;
}

PaError DataPortAudio::openStream(int iDev, int oDev, int iChans, int oChans, int sampleRate, int frames) {
  inputChannels = iChans;
  outputChannels = oChans;
  outputting = false;

  char strbuf[256];
  for (int i=0; i<16; i++)
    buffers[i]->destroy();
  for (int i=0; i<iChans; i++) {
    snprintf(strbuf, 256, "PortAudio Input Channel %d", i+1);
    buffers[i]->init(65536, sampleRate, strbuf);
  }

  // if (iDev > inputDevicesInternal.size() || oDev > outputDevicesInternal.size()) {
  //   printf(ERROR_MSG "PA: invalid device number!" MSG_END);
  //   return paInvalidDevice;
  // }

  streamParamsI.device = iDev;
  streamParamsI.channelCount = iChans;
  streamParamsI.sampleFormat = paFloat32;
  streamParamsI.suggestedLatency = Pa_GetDeviceInfo(streamParamsI.device)->defaultLowInputLatency;
  streamParamsI.hostApiSpecificStreamInfo = NULL;

  if (oDev!=-1) {
    streamParamsO.device = oDev;
    streamParamsO.channelCount = oChans;
    streamParamsO.sampleFormat = paFloat32;
    streamParamsO.suggestedLatency = Pa_GetDeviceInfo(streamParamsO.device)->defaultHighOutputLatency;
    streamParamsO.hostApiSpecificStreamInfo = NULL;
    outputting = true;
  }
  int ret = Pa_OpenStream(
    &stream,
    &streamParamsI,
    outputting?&streamParamsO:NULL,
    sampleRate,
    frames,
    paClipOff|paDitherOff,
    &DataPortAudio::audioCallback,
    this
  );
  printf(MISC_MSG "Pa_OpenStream return: %d" MSG_END, ret);
  return ret;
}

void DataPortAudio::activate() {
  if ((state&DRIVERSTATE_ACTIVE))  {
    doPlay(false);
    deactivate();
    if (state&DRIVERSTATE_ERROR) {
      printf(ERROR_MSG "PA: failed to deactivate device!" MSG_END);
      return;
    }
  }
  if (!(state&(DRIVERSTATE_OK|DRIVERSTATE_READY)))  {
    printf(ERROR_MSG "PA: cannot activate!" MSG_END);
    state |= DRIVERSTATE_ERROR;
    return;
  }
  if (inputDeviceP == paNoDevice) {
    printf(ERROR_MSG "PA: no input device!" MSG_END);
    state |= DRIVERSTATE_ERROR;
    return;
  }

  printf(INFO_MSG "opening pa stream..." MSG_END);
  e = openStream(inputDevicesInternal[inputDeviceP],
    outputDevicesInternal[outputDeviceP],
    inputChannelsP,
    outputChannelsP,
    sampleRates[sampleRateP+1],
    frameSizes[frameSizeP+1]);
  if (e!=paNoError) {
    printf(ERROR_MSG "PA: failed to open stream! %s" MSG_END, Pa_GetErrorText(e));
    // state |= DRIVERSTATE_ERROR;
    // return;
  }

  // if (e) printf(ERROR_MSG "opening stream failed! %d: %s" MSG_END, e, Pa_GetErrorText(e));
  switch (e) {
    case paInvalidDevice:
      printf(INFO_MSG "trying default devices..." MSG_END);
      e = openStream(Pa_GetDefaultInputDevice(),
        outputDevicesInternal[outputDeviceP]==-1?-1:Pa_GetDefaultOutputDevice(),
        inputChannelsP,
        outputChannelsP,
        sampleRates[sampleRateP+1],
        frameSizes[frameSizeP+1]);
      break;
    case paInvalidChannelCount: {
      // TODO: handle chan count switching
      // int outChans=conf->outputDevice>0?Pa_GetDeviceInfo(conf->outputDevice)->maxOutputChannels:0;
      printf(INFO_MSG "trying preferred channel count..." MSG_END);
      e = openStream(Pa_GetDefaultInputDevice(),
        outputDevicesInternal[outputDeviceP]==-1?-1:Pa_GetDefaultOutputDevice(),
        inputChannelsP,
        outputChannelsP,
        sampleRates[sampleRateP+1],
        frameSizes[frameSizeP+1]);
      break;
    }
    case paInvalidSampleRate:
      printf(INFO_MSG "trying default sample rate..." MSG_END);
      e = openStream(inputDevicesInternal[inputDeviceP],
        outputDevicesInternal[outputDeviceP],
        inputChannelsP,
        outputChannelsP,
        Pa_GetDeviceInfo(inputDevicesInternal[inputDeviceP])->defaultSampleRate,
        frameSizes[frameSizeP+1]);
      break;
    case paNoError:
    default: break;
  }

  if (e!=paNoError) {
    printf(ERROR_MSG "PA: NOOOOOOO!!! %s" MSG_END, Pa_GetErrorText(e));
    state |= DRIVERSTATE_ERROR;
    return;
  }

  state |= DRIVERSTATE_ACTIVE;
}

void DataPortAudio::deactivate() {
  if (!(state&DRIVERSTATE_ACTIVE)) {
    printf(INFO_MSG "PA: cannot decativate while inactive!" MSG_END);
    return;
  }
  doPlay(false);
  Pa_CloseStream(stream);
  state &=~DRIVERSTATE_ACTIVE;
}

void DataPortAudio::doPlay(bool play) {
  if (!(state&DRIVERSTATE_ACTIVE)) {
    // state |= DRIVERSTATE_ERROR;
    return;
  }
  if (play) {
    if (state&DRIVERSTATE_PLAY) return;
    e = Pa_StartStream(stream);
    if (e!=paNoError) {
      printf(ERROR_MSG "PA: failed to start stream! %s" MSG_END, Pa_GetErrorText(e));
      state |= DRIVERSTATE_ERROR;
      return;
    }
    state |= DRIVERSTATE_PLAY;
  } else {
    if (!(state&DRIVERSTATE_PLAY)) return;
    e = Pa_StopStream(stream);
    if (e!=paNoError) {
      printf(ERROR_MSG "PA: failed to stop stream! %s" MSG_END, Pa_GetErrorText(e));
      state |= DRIVERSTATE_ERROR;
      return;
    }
    state &=~DRIVERSTATE_PLAY;
  }
}

int DataPortAudio::audioCallback(
    const void *inputBuffer, void *outputBuffer,
    unsigned long int framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData) {
  DataPortAudio* self = (DataPortAudio*)userData;
  const float* input = (const float*)inputBuffer;
  float* output = (float*)outputBuffer;
  float ZERO=0.0f;
  if (input == NULL) {
    for (unsigned long int i=0; i<framesPerBuffer; i++) {
      for (int j=0; j<self->inputChannels; j++) {
        DataBuffer* buf = self->buffers[j];
        if (buf == NULL) continue;
        if (buf->getBuffer() == NULL) continue;
        buf->write(&ZERO);
      }
    }
    return paContinue;
  }
  for (unsigned long int i=0; i<framesPerBuffer; i++) {
    for (int j=0; j<self->inputChannels; j++) {
      DataBuffer* buf = self->buffers[j];
      if (buf == NULL) continue;
      if (buf->getBuffer() == NULL) continue;
      buf->write((void*)input++);
    }
  }

  return paContinue;
}

int DataPortAudio::enumerateDevices() {
  inputDevices.clear();
  outputDevices.clear();
  inputDevicesInternal.clear();
  outputDevicesInternal.clear();
  e = Pa_GetDeviceCount();
  if (e < 1) {
    printf(ERROR_MSG "PA: error when getting device count! %s" MSG_END, Pa_GetErrorText(e));
    return 1;
  }

  inputDevicesInternal.push_back(Pa_GetDefaultInputDevice());
  inputDevices.push_back("Default input");
  outputDevicesInternal.push_back(-1);
  outputDevices.push_back("No output");
  outputDevicesInternal.push_back(Pa_GetDefaultOutputDevice());
  outputDevices.push_back("Default output");

  char strbuf[512];
  const int count = Pa_GetDeviceCount();
  for (int i=0; i<count; i++) {
    const PaDeviceInfo* dev = Pa_GetDeviceInfo(i);
    if (dev->maxInputChannels>0) {
      snprintf(strbuf, 512, "%s | %s", Pa_GetHostApiInfo(dev->hostApi)->name, dev->name);
      inputDevicesInternal.push_back(i);
      inputDevices.push_back(string(strbuf));
    }
    if (dev->maxOutputChannels>0) {
      snprintf(strbuf, 512, "%s | %s", Pa_GetHostApiInfo(dev->hostApi)->name, dev->name);
      outputDevicesInternal.push_back(i);
      outputDevices.push_back(string(strbuf));
    }
  }
  return 0;
}

void DataPortAudio::destroy() {
  if (paInitSuccess) {
    if (state&DRIVERSTATE_ACTIVE) {
      if (state&DRIVERSTATE_PLAY) doPlay(false);
      deactivate();
    }
    Pa_Terminate();
    state&=~DRIVERSTATE_READY;
  }
  for (int i=0; i<config.size(); i++) config[i].destroy();
  config.clear();
  for (int i=0; i<buffers.size(); i++) buffers[i]->destroy();
  buffers.clear();
}

DataPortAudio::~DataPortAudio() {
  inputDevices.clear();
  outputDevices.clear();
  inputDevicesInternal.clear();
  outputDevicesInternal.clear();
}
