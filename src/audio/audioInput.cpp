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
  audioBuffer *buffer = (audioBuffer*)userData;
  const float *datRPtr = (const float*)inputBuffer;
  float *datWPtr = &buffer->data[buffer->index*2]; //replace 2!!!
  long framesToCalc,i;
  int finished;

  unsigned long framesLeft = buffer->size-buffer->index;

  (void) outputBuffer;
  (void) timeInfo;
  (void) statusFlags;
  (void) userData;
  if( framesLeft < framesPerBuffer ) {
    framesToCalc = framesLeft;
    finished = paComplete;
  }
  else {
    framesToCalc = framesPerBuffer;
    finished = paContinue;
  }
  if( inputBuffer == NULL ) {
    for( i=0; i<framesToCalc; i++ ) {
      *datWPtr++ = 0;  /* left */
      *datWPtr++ = 0;  /* right */
    }
  }
  else {
    for( i=0; i<framesToCalc; i++ ) {
      *datWPtr++ = *datRPtr++;  /* left */
      *datWPtr++ = *datRPtr++;  /* right */
    }
  }
  return finished;
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
    &buffer
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

float *AudioInput::getData() {
  return buffer.data;
}