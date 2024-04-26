#include "audio.h"

AudioInput::AudioInput() {
  conf.channels=2;
  conf.sampleRate=44100;
  conf.frameSize=512;
}

int AudioInput::bufferWriteCallback(const void *inputBuffer, void *outputBuffer,
                                           unsigned long framesPerBuffer,
                                           const PaStreamCallbackTimeInfo* timeInfo,
                                           PaStreamCallbackFlags statusFlags,
                                           void *userData) {
  return 0;
}

int AudioInput::init() {
  streamParams.device = Pa_GetDefaultInputDevice();
  if (streamParams.device == paNoDevice) return -1;
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
    bufferWriteCallback,
    NULL
  );

  return 0;
}
AudioInput::~AudioInput() {}