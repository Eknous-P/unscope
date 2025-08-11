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

#include "audio.h"
#include "audio_pa.h"

USCAudio::USCAudio(AudioConfig* conf, nint bufSize) {
  devices.clear();
  config = conf;

  buffer.size = bufSize;
  buffer.chans = conf->outputChannels;
  buffer.data = new AudioSample* [buffer.chans];
  NEW_DOUBLE_PTR(buffer.data, AudioSample, buffer.size, buffer.chans)

  loopbackVolume = 0.0f;

  audioIO = NULL;
}

int USCAudio::internalAudioCallback(void* self, const void* audioInput, void* audioOutput, nint frames) {
  USCAudio* s = (USCAudio*)self;
  if (!s->buffer.data) return 1;

  const AudioSample* audIn = (const float*)audioInput;
  AudioSample* audOut = (float*)audioOutput;
  if (!audIn) return 2;

  const nint bufSize = s->buffer.size;
  const unsigned char chans = s->buffer.chans;

  if (!audioInput) {
    FOR_RANGE(chans) {
      memset(s->buffer.data[z], 0, bufSize * sizeof(AudioSample));
    }
    return 0;
  }
  FOR_RANGE(chans) {
    memcpy(s->buffer.data[z], s->buffer.data[z] + frames, (bufSize - frames) * sizeof(AudioSample));
  }
  for (nint i = bufSize - frames; i < bufSize; i++) {
    FOR_RANGE(chans) {
      AudioSample value = *audIn++;
      s->buffer.data[z][i] = value;
      if (audOut) *audOut++ = s->loopbackVolume * value;
    }
  }

  return 0;
}

AudioConfig* USCAudio::getConfig() {
  return config;
}

AudioBuffer* USCAudio::getBuffer() {
  return &buffer;
}

vector<AudioDevice>* USCAudio::getDevices() {
  return &devices;
}

int(*USCAudio::getCallbackFunction())(void*, const void*, void*, nint) {
  return internalAudioCallback;
}

int USCAudio::initIO(AudioDrivers d) {
  switch (d) {
    case AUDIO_PORTAUDIO:
      audioIO = new Audio_PA;
      break;
    case AUDIO_DUMMY:
    default:
      audioIO = new AudioIO;
      break;
  }
  if (!audioIO) return 1;
  audioIO->setup(this);
  return 0;
}

void USCAudio::setLoopback(float v) {
  loopbackVolume = v;
}

int USCAudio::getDefaultInputDevice() {
  if (!audioIO) return -1;
  return audioIO->getDefaultInputDevice();
}

int USCAudio::getDefaultOutputDevice() {
  if (!audioIO) return -1;
  return audioIO->getDefaultOutputDevice();
}

const char* USCAudio::getLastIOError() {
  if (!audioIO) return NULL;
  return audioIO->getLastError();
}

int USCAudio::initAudio() {
  if (!audioIO) return 1;
  return audioIO->init();
}

int USCAudio::startAudio() {
  if (!audioIO) return 1;
  return audioIO->start();
}

int USCAudio::stopAudio() {
  if (!audioIO) return 1;
  return audioIO->stop();
}

int USCAudio::deinitAudio() {
  if (!audioIO) return 1;
  return audioIO->deinit();
}

int USCAudio::deinitIO() {
  if (!audioIO) return 1;
  delete audioIO;
  audioIO = NULL;
  return 0;
}

int USCAudio::getAvailDevices() {
  devices.clear();
  devices.push_back(AudioDevice(DIR_OUT, -1, 0, "No Output"));
  if (!audioIO) return -1;
  return audioIO->getAvailDevices();
}

AudioSample** USCAudio::getAudioBuffer() {
  return buffer.data;
}

bool USCAudio::isOutputting() {
  if (!audioIO) return false;
  return audioIO->isOutputting();
}

USCAudio::~USCAudio() {
  DELETE_DOUBLE_PTR_ARR(buffer.data, buffer.chans)
}
