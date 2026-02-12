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

const int DataPortAudio::getFlags() {
  return DRIVERFLAG_OUTPUT|DRIVERFLAG_INPUT;
}

int DataPortAudio::setup(USCData* p) {
  parent = p;
  running = false;
  paInitSuccess = false;

  buffers = {};
  inputDevices = {};
  outputDevices = {};
  outputting = false;

  inputChannels = outputChannels = 0;

  config = {
    Parameter(PARAM_INPUTINT, false, "input channels", (void*)paramChannelsLimits),
    Parameter(PARAM_INPUTINT, false, "output channels", (void*)paramChannelsLimits),
    Parameter(PARAM_COMBO_INT, false, "sample rate", (void*)sampleRates),
    Parameter(PARAM_COMBOV_STR, false, "input device", &inputDevices),
    Parameter(PARAM_COMBOV_STR, false, "output device", &outputDevices),
    Parameter(PARAM_COMBO_INT, false, "frame size", (void*)frameSizes),
  };

  config[0].setValue((int)2);
  config[1].setValue((int)2);

  e = Pa_Initialize();
  if (e!=paNoError) return e;
  else paInitSuccess = true;
  return 0;
}

void DataPortAudio::openStream(int iDev, int oDev, int iChans, int oChans, int sampleRate, int frames) {
  inputChannels = iChans;
  outputChannels = oChans;
  outputting = false;

  for (int i=0 ;i<buffers.size(); i++) {
    delete buffers[i];
  }
  buffers.clear();
  char strbuf[256];
  for (int i=0; i<iChans; i++) {
    DataBuffer* newBuf = new DataBuffer_Float;
    snprintf(strbuf, 256, "PortAudio Input Channel %d", i+1);
    newBuf->init(65536, sampleRate, strbuf);
    buffers.push_back(newBuf);
  }

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
  e = Pa_OpenStream(
    &stream,
    &streamParamsI,
    outputting?&streamParamsO:NULL,
    sampleRate,
    frames,
    paClipOff,
    &DataPortAudio::audioCallback,
    this
  );
}

void DataPortAudio::closeStream() {
  if (running) {
    running = false;
    e = Pa_CloseStream(stream);
  }
}

#define inputDeviceN config[3].getValue<int>()
#define outputDeviceN config[4].getValue<int>()

int DataPortAudio::init() {
  if (running) return -1;
  if (inputDeviceN == paNoDevice) return 1;

  printf(INFO_MSG "opening pa stream..." MSG_END);
  openStream(inputDevicesInternal[inputDeviceN],
    outputDevicesInternal[outputDeviceN],
    config[0].getValue<int>(),
    config[1].getValue<int>(),
    sampleRates[config[2].getValue<int>()+1],
    frameSizes[config[5].getValue<int>()+1]);

  if (e) printf(ERROR_MSG "opening stream failed! %d: %s" MSG_END, e, Pa_GetErrorText(e));
  switch (e) {
    case paInvalidDevice:
      printf(INFO_MSG "trying default devices..." MSG_END);
      openStream(Pa_GetDefaultInputDevice(),
        outputDevicesInternal[outputDeviceN]==-1?-1:Pa_GetDefaultOutputDevice(),
        config[0].getValue<int>(),
        config[1].getValue<int>(),
        sampleRates[config[2].getValue<int>()+1],
        frameSizes[config[5].getValue<int>()+1]);
      break;
    case paInvalidChannelCount: {
      // TODO: handle chan count switching
      // int outChans=conf->outputDevice>0?Pa_GetDeviceInfo(conf->outputDevice)->maxOutputChannels:0;
      printf(INFO_MSG "trying preferred channel count..." MSG_END);
      // openStream(conf->inputDevice,
      //   conf->outputDevice,
      //   Pa_GetDeviceInfo(conf->inputDevice)->maxInputChannels,
      //   outChans,
      //   conf->sampleRate,
      //   conf->frameSize);
      config[0].setValue(2);
      openStream(Pa_GetDefaultInputDevice(),
        outputDevicesInternal[outputDeviceN]==-1?-1:Pa_GetDefaultOutputDevice(),
        config[0].getValue<int>(),
        config[1].getValue<int>(),
        sampleRates[config[2].getValue<int>()+1],
        frameSizes[config[5].getValue<int>()+1]);
      break;
    }
    case paInvalidSampleRate:
      printf(INFO_MSG "trying default sample rate..." MSG_END);
      openStream(inputDevicesInternal[inputDeviceN],
        outputDevicesInternal[outputDeviceN],
        config[0].getValue<int>(),
        config[1].getValue<int>(),
        Pa_GetDeviceInfo(inputDevicesInternal[inputDeviceN])->defaultSampleRate,
        frameSizes[config[5].getValue<int>()+1]);
      break;
    case paNoError:
    default: break;
  }

  if (e!=paNoError) {
    printf(ERROR_MSG "NOOOOOOO!!!" MSG_END);
    return 2;
  }

  return 0;
}

#undef inputDeviceN
#undef outputDeviceN

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

int DataPortAudio::start() {
  if (running) return -1;
  e = Pa_StartStream(stream);
  switch (e) {
    case paNoError:
      running = true;
      return 0;
    default: return 1;
  }
}

int DataPortAudio::stop() {
  if (!running) return -1;
  if (Pa_IsStreamActive(stream)) e = Pa_StopStream(stream);
  switch (e) {
    case paNoError:
      running = false;
      return 0;
    default: return 1;
  }
}

int DataPortAudio::enumerateDevices() {
  inputDevices.clear();
  outputDevices.clear();
  inputDevicesInternal.clear();
  outputDevicesInternal.clear();
  if (Pa_GetDeviceCount() < 1) return 1;

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
      snprintf(strbuf, 512, "%d: %s | %s", i, Pa_GetHostApiInfo(dev->hostApi)->name, dev->name);
      inputDevicesInternal.push_back(i);
      inputDevices.push_back(string(strbuf));
    }
    if (dev->maxOutputChannels>0) {
      snprintf(strbuf, 512, "%d: %s | %s", i, Pa_GetHostApiInfo(dev->hostApi)->name, dev->name);
      outputDevicesInternal.push_back(i);
      outputDevices.push_back(string(strbuf));
    }
  }
  return 0;
}

string DataPortAudio::getLastError() {
  lastErrorStr = Pa_GetErrorText(e);
  return lastErrorStr;
}

int DataPortAudio::deinit() {
  closeStream();
  buffers.clear();
  if (paInitSuccess) Pa_Terminate();
  return 0;
}

const char* DataPortAudio::getName() {
  return "PortAudio Driver";
}

DataPortAudio::~DataPortAudio() {
  inputDevices.clear();
  outputDevices.clear();
  inputDevicesInternal.clear();
  outputDevicesInternal.clear();
}
