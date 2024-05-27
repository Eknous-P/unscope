#include "audio.h"

AudioProcess::AudioProcess(unsigned int bufferSizeDef, unsigned char chans) {
  channels = chans;
  dataSize = bufferSizeDef;

  dataIn = new float*[channels];
  dataOut = new float*[channels];
  alignRamp = new float*[channels];
  for (i = 0; i < channels; i++) {
    dataOut[i] = new float[dataSize];
    alignRamp[i] = new float[dataSize];

    memset(dataOut[i], 0, dataSize*sizeof(float));
    memset(alignRamp[i], 0, dataSize*sizeof(float));
  }
  i = 0;
}

AudioProcess::~AudioProcess() {
  if (dataOut) {
    for (unsigned char i = 0; i < channels; i++) {
      if (dataOut[i]) {
        delete[] dataOut[i];
        dataOut[i] = NULL;
      }
    }
    delete[] dataOut;
    dataIn = NULL;
  }
  if (alignRamp) {
    for (unsigned char i = 0; i < channels; i++) {
      if (alignRamp[i]) {
        delete[] alignRamp[i];
        alignRamp[i] = NULL;
      }
    }
    delete[] alignRamp;
    alignRamp = NULL;
  }
  if (dataIn) {
    delete[] dataIn;
    dataIn = NULL;
  }
}

void AudioProcess::writeDataIn(float *d, unsigned char chan) {
  dataIn[chan] = d;
}

float *AudioProcess::getDataOut(unsigned char chan) {
  return dataOut[chan];
}

void AudioProcess::derive() {
  dataOut[i] = 0;
  for (unsigned char j = 0; j < channels; j++) {
    for (i = 1; i < dataSize; i++) {
      dataOut[j][i] = dataIn[j][i] - dataIn[j][i-1];
    }
  }
  i = 0;
}

void AudioProcess::integrate() {
  dataOut[0] = dataIn[0];
  for (unsigned char j = 0; j < channels; j++) {
    for (i = 1; i < dataSize; i++) {
      dataOut[j][i] = dataOut[j][i-1] + dataIn[j][i];
    }
  }
  i = 0;
}

float *AudioProcess::alignWave(unsigned char chan, float trigger, unsigned long int waveLen, long int offset, bool edge=false) {
  memset(alignRamp[chan],0.0f,sizeof(float)*dataSize);
  // i = dataSize - waveLen + offset;
  // i = waveLen;
  // while ((dataIn[chan][i] < trigger) == (edge == 0)) {
  //   i--;
  // }
  // alignRamp[i] = (float)(65536-i+offset)/waveLen;
  // for (i++;i<dataSize;i++) {
  //   alignRamp[i] = alignRamp[i-1] + delta/dataSize;
  // }

  float delta = (float)dataSize/((float)waveLen)/(float)dataSize;
  alignRamp[chan][dataSize-1] = 1.0f;
  for (i = dataSize-2; i > 0; i--) {
    alignRamp[chan][i] = alignRamp[chan][i+1] - 2*delta;
  }
  alignRamp[chan][0] = -1.0f;

  i = 0;
  return alignRamp[chan];
}