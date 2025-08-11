/*
unscope - an audio oscilloscope
Copyright (C) 2025 Eknous

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

#include "audio_pa.h"

int Audio_PA::setup(USCAudio* p) {
  parent = p;
  conf = p->getConfig();
  buf  = p->getBuffer();
  devs = p->getDevices();
  callbackFunction = p->getCallbackFunction();

  running = false;
  outputting = false;

  if (Pa_Initialize()!=paNoError) return 1;
  return 0;
}

void Audio_PA::openStream(int iDev, int oDev, int iChans, int oChans, int sampleRate, int frames) {
  outputting = false;

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
    &Audio_PA::audioCallback,
    this
  );
}

void Audio_PA::closeStream() {
  running = false;
  e = Pa_CloseStream(stream);
}

int Audio_PA::init() {
  if (running) return -1;
  if (conf->inputDevice == paNoDevice) return 1;

  printf(INFO_MSG "opening pa stream..." MSG_END);
  openStream(conf->inputDevice,
    conf->outputDevice,
    conf->inputChannels,
    conf->outputChannels,
    conf->sampleRate,
    conf->frameSize);

  if (e) printf(ERROR_MSG "opening stream failed! %d: %s" MSG_END, e, Pa_GetErrorText(e));
  switch (e) {
    case paInvalidDevice:
      printf(INFO_MSG "trying default devices..." MSG_END);
      openStream(Pa_GetDefaultInputDevice(),
        (conf->outputDevice==-1)?-1:Pa_GetDefaultOutputDevice(),
        conf->inputChannels,
        conf->outputChannels,
        conf->sampleRate,
        conf->frameSize);
      break;
    case paInvalidChannelCount:
      printf(INFO_MSG "trying preferred channel count..." MSG_END);
      openStream(conf->inputDevice,
        conf->outputDevice,
        Pa_GetDeviceInfo(conf->inputDevice)->maxInputChannels,
        Pa_GetDeviceInfo(conf->outputDevice)->maxOutputChannels,
        conf->sampleRate,
        conf->frameSize);
      break;
    case paInvalidSampleRate:
      printf(INFO_MSG "trying default sample rate..." MSG_END);
      openStream(conf->inputDevice,
        conf->outputDevice,
        conf->inputChannels,
        conf->outputChannels,
        (int)Pa_GetDeviceInfo(conf->inputDevice)->defaultSampleRate,
        conf->frameSize);
      break;
    case paNoError:
    default: break;
  }

  if (e!=paNoError) return 2;

  return 0;
}

int Audio_PA::audioCallback(
    const void *inputBuffer, void *outputBuffer,
    unsigned long int framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData) {
  return ((Audio_PA*)userData)->callbackFunction(((Audio_PA*)userData)->parent, (void*)inputBuffer, outputBuffer, framesPerBuffer);
}

int Audio_PA::start() {
  if (running) return -1;
  e = Pa_StartStream(stream);
  switch (e) {
    case paNoError:
      running = true;
      return 0;
    default: return 1;
  }
}

int Audio_PA::stop() {
  if (!running) return -1;
  if (Pa_IsStreamActive(stream)) e = Pa_StopStream(stream);
  switch (e) {
    case paNoError:
      running = false;
      return 0;
    default: return 1;
  }
}

bool Audio_PA::isRunning() {
  return running;
}

bool Audio_PA::isOutputting() {
  return outputting;
}

int Audio_PA::getAvailDevices() {
  if (Pa_GetDeviceCount() < 1) return 0;
  devs->push_back(AudioDevice(DIR_IN,  Pa_GetDefaultInputDevice(),  Pa_GetDefaultHostApi(), "Default input"));
  devs->push_back(AudioDevice(DIR_OUT, Pa_GetDefaultOutputDevice(), Pa_GetDefaultHostApi(), "Default output"));
  for (int i = 0; i < Pa_GetDeviceCount(); i++) {
    const PaDeviceInfo* d = Pa_GetDeviceInfo(i);

    unsigned char dir =
      ((d->maxInputChannels > 0) ?DIR_IN :0)|
      ((d->maxOutputChannels > 0)?DIR_OUT:0);

    char devName[256];
    snprintf(devName, 256, "%.2d: %s | %s",
      i, Pa_GetHostApiInfo(d->hostApi)->name, d->name);

    devs->push_back(AudioDevice(dir, i, d->hostApi, devName));
  }
  return (int)devs->size();
}

int Audio_PA::getDefaultInputDevice() {
  return Pa_GetDefaultInputDevice();
}

int Audio_PA::getDefaultOutputDevice() {
  return Pa_GetDefaultOutputDevice();
}

const char* Audio_PA::getLastError() {
  return Pa_GetErrorText(e);
}

int Audio_PA::deinit() {
  closeStream();
  return 0;
}
