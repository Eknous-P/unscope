#include "audio.h"

inline float clamp(float a) {
  if (a > 1.0f) return 1.0f;
  if (a < -1.0f) return -1.0f;
  return a;
}

AudioProcess::AudioProcess(unsigned int bufferSizeDef, unsigned char chans) {
  channels = chans;
  dataSize = bufferSizeDef;

  triggered = false;

  dataIn = new float*[channels];
  dataOut = new float*[channels];
  alignRamp = new float[dataSize];
  memset(alignRamp, 0, dataSize*sizeof(float));
  for (i = 0; i < channels; i++) {
    dataOut[i] = new float[dataSize];
    memset(dataOut[i], 0, dataSize*sizeof(float));
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
  memset(alignRamp,-1.0f,sizeof(float)*dataSize);

  unsigned int triggerPoint = 0;
  bool triggerLow = false, triggerHigh = false;
  i = dataSize - waveLen;
  while (i != 0 && i > (dataSize - 2*waveLen)) {
    triggered = (triggerLow && triggerHigh);
    i--;
    if (dataIn[chan][i] < trigger) {
      triggerLow = true;
      if (triggered && edge) break;
    }
    if (dataIn[chan][i] > trigger) {
      triggerHigh = true;
      if (triggered && !edge) break;
    }
  }
  triggerPoint = i;

  float delta = 0;

  if (triggered) {
    delta = 2.0f/(float)(waveLen);
    for (;i < dataSize; i++) {
      alignRamp[i-offset] = -1.0f + delta*(i - triggerPoint);
    }
  } else {
    delta = ((float)dataSize/((float)waveLen))/(float)dataSize;
    alignRamp[dataSize-1] = 1.0f;
    for (i = dataSize-2; i > 0; i--) {
      alignRamp[i] = clamp(alignRamp[i+1] - 2*delta);
    }
    alignRamp[0] = -1.0f;
  }

  i = 0;
  return alignRamp;
}

bool AudioProcess::didTrigger() {
  return triggered;
}