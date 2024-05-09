#include "audio.h"

AudioInput::AudioInput() {
  conf.channels=2;
  conf.sampleRate=44100;
  conf.frameSize=512;
}

int AudioInput::_PaCallback(
  const void *input, void *output,
  unsigned long frameCount,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData )
{
  return ((AudioInput*)userData)->bufferGetCallback(input, output, frameCount, timeInfo, statusFlags);
}

int AudioInput::bufferGetCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags) {
    
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
    _PaCallback,
    this
  );

  if (err!=paNoError) return -2;

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