#include "audio.h"

AudioInput::AudioInput() {
  isGood = Pa_Initialize() == paNoError;
  running=false;
  conf.channels=1;
  conf.sampleRate=48000;
  conf.frameSize=BUFFER_SIZE;

  conf.device=0;

  buffer.size=BUFFER_SIZE;
  buffer.data=new float[buffer.size];
}

int AudioInput::_PaCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long int framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData ) {
    return ((AudioInput*)userData)->bufferGetCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int AudioInput::bufferGetCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long int framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags) {
    const float *audIn = (const float*)inputBuffer;
  
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    if (inputBuffer==NULL) {
      for (buffer.index = 0; buffer.index < framesPerBuffer; buffer.index++) {
        buffer.data[buffer.index] = 0;
      }
    } else {
      for (buffer.index = 0; buffer.index < framesPerBuffer; buffer.index++) {
        buffer.data[buffer.index] = *audIn++;
      }
    }
    return paContinue;
}

int AudioInput::init(PaDeviceIndex dev) {
  if (!isGood) return NOGOOD;
  if (running) return NOERR;
  int nDevs = Pa_GetDeviceCount();
  if (nDevs == 0) return NODEVS;
  if (nDevs < 0) return nDevs;

  streamParams.device = dev;
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
    &AudioInput::_PaCallback,
    this
  );
  Pa_Sleep(100);

  if (err!=paNoError) return err;

  err = Pa_StartStream(stream);
  
  conf.device=dev;
  running = err==paNoError;
  return err;
}

int AudioInput::stop() {
  return Pa_CloseStream(stream);
}

AudioInput::~AudioInput() {
  if (isGood) Pa_Terminate();
  if (buffer.data) {
    delete[] buffer.data;
    buffer.data=NULL;
  }
}

float *AudioInput::getData() {
  return buffer.data;
}

unsigned long int AudioInput::getDataSize() {
  return buffer.size;
}

const PaDeviceInfo* AudioInput::getDeviceInfo() {
  return Pa_GetDeviceInfo(conf.device);
}