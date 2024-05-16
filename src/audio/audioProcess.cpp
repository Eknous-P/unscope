#include "audio.h"

AudioProcess::AudioProcess() {
  // dataIn = new float[BUFFER_SIZE];
  dataOut = new float[BUFFER_SIZE];
  dataSize = BUFFER_SIZE;
  i = 0;
}

AudioProcess::~AudioProcess() {
  // delete[] dataIn;
  delete[] dataOut;
  dataIn = NULL; dataOut = NULL;
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
  for (i=1; i<dataSize; i++) {
    dataOut[i] = dataIn[i] - dataIn[i-1];
  }
}

void AudioProcess::integrate() {
  dataOut[0] = dataIn[0];
  for (i=1; i<dataSize; i++) {
    dataOut[i] = dataOut[i-1] + dataIn[i];
  }
}