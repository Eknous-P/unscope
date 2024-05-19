#include "audio.h"

AudioProcess::AudioProcess(unsigned int bufferSizeDef) {
  dataOut = new float[bufferSizeDef];
  alignRamp = new float[bufferSizeDef];
  dataSize = bufferSizeDef;
  i = 0;
}

AudioProcess::~AudioProcess() {
  delete[] dataOut;
  delete[] alignRamp;
  dataIn = NULL; dataOut = NULL;
  alignRamp = NULL;
}

void AudioProcess::writeDataIn(float *d) {
  dataIn = d;
}

float *AudioProcess::getDataOut() {
  return dataOut;
}

float *AudioProcess::getDataIn() {
  return dataIn;
}

void AudioProcess::derive() {
  dataOut[i] = 0;
  for (i = 1; i < dataSize; i++) {
    dataOut[i] = dataIn[i] - dataIn[i-1];
  }
  i = 0;
}

void AudioProcess::integrate() {
  dataOut[0] = dataIn[0];
  for (i = 1; i < dataSize; i++) {
    dataOut[i] = dataOut[i-1] + dataIn[i];
  }
  i = 0;
}

float *AudioProcess::alignWave(float trigger, unsigned long int waveLen, long int offset, bool edge=false) {
  i = dataSize - waveLen + offset;
  // (void)edge;
  // while (dataIn[i] - trigger < 0) i++;
  // alignRamp[i] = 0;
  // for (i=0;i<dataSize;i++) {
  //   alignRamp[i] = alignRamp[i-1] + 1;
  // }

  for (i = 0; i < dataSize; i++) {
    alignRamp[i] = 2*(float)i/dataSize - 1;
  }
  i = 0;
  return alignRamp;
}