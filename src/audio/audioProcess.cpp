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
  memset(alignRamp,0.0f,sizeof(float)*dataSize);
  // i = dataSize - waveLen + offset;
  // i = waveLen;
  // while ((dataIn[i] < trigger) == (edge == 0)) {
  //   i--;
  // }
  // alignRamp[i] = (float)(65536-i+offset)/waveLen;
  // for (i++;i<dataSize;i++) {
  //   alignRamp[i] = alignRamp[i-1] + delta/dataSize;
  // }

  // float delta = (float)((float)dataSize-(float)waveLen+1)/(float)dataSize/48000;
  float delta = (float)dataSize/((float)waveLen)/(float)dataSize;
  alignRamp[dataSize-1] = 1.0f;
  for (i = dataSize-2; i > 0; i--) {
    alignRamp[i] = alignRamp[i+1] - 2*delta;
  }
  alignRamp[0] = -1.0f;

  i = 0;
  return alignRamp;
}