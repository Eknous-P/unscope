#include "audio.h"
#include <iostream>

AudioInput::AudioInput() {
  Pa_Initialize();
  conf.channels=1;
  conf.sampleRate=48000;
  conf.frameSize=8192;

  buffer.size=8192;
  for (buffer.index=0; buffer.index<buffer.size; buffer.index++) {
    buffer.data[buffer.index]==(float)(buffer.index)/8192;
  }
}

int AudioInput::_PaCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData )
{
  return ((AudioInput*)userData)->bufferGetCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int AudioInput::bufferGetCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags) {
    const float* audIn = (const float*)inputBuffer;
  
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    if (inputBuffer==NULL) {
      for (buffer.index = 0; buffer.index < framesPerBuffer; buffer.index++) {
        *buffer.data++ = 1.0f;
      }
    } else {
      for (buffer.index = 0; buffer.index < framesPerBuffer; buffer.index++) {
        *buffer.data++ = *audIn++;
      }
    }
    return paContinue;
}

int AudioInput::init() {
  if (Pa_GetDeviceCount() == 0) return NODEVS;
  if (Pa_GetDeviceCount() < 0) return Pa_GetDeviceCount();
  streamParams.device = Pa_GetDefaultInputDevice();
  if (streamParams.device == paNoDevice) return NODEV;
  streamParams.channelCount = conf.channels;
  streamParams.sampleFormat = paFloat32;
  streamParams.suggestedLatency = Pa_GetDeviceInfo(streamParams.device)->defaultLowInputLatency;
  streamParams.hostApiSpecificStreamInfo = NULL;

  err = Pa_OpenStream(
    &stream,
    &streamParams,
    NULL,
    conf.sampleRate,
    conf.frameSize,
    paClipOff,
    _PaCallback,
    this
  );

  if (err!=paNoError) return NOSTART;

  err = Pa_StartStream(stream);

  return err;
}

int AudioInput::stop() {
  return Pa_CloseStream(stream);
}

AudioInput::~AudioInput() {
  Pa_Terminate();
  if (buffer.data) free(buffer.data);
}

void *AudioInput::getData() {
  return &buffer.data;
}

unsigned long AudioInput::getDataSize() {
  return buffer.size;
}