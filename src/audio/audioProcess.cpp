#include "audio.h"

USCAudioProcess::USCAudioProcess(unsigned int bufferSizeDef, unsigned char chans) {
  channels = chans;
  dataSize = bufferSizeDef;

  dataIn = new float*[channels];
  dataOut = new float*[channels];
  for (i = 0; i < channels; i++) {
    dataOut[i] = new float[dataSize];
    memset(dataOut[i], 0, dataSize*sizeof(float));
  }
  i = 0;
}

USCAudioProcess::~USCAudioProcess() {
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
  if (dataIn) {
    delete[] dataIn;
    dataIn = NULL;
  }
}

void USCAudioProcess::writeDataIn(float *d, unsigned char chan) {
  dataIn[chan] = d;
}

float *USCAudioProcess::getDataOut(unsigned char chan) {
  return dataOut[chan];
}

void USCAudioProcess::derive() {
  dataOut[i] = 0;
  for (unsigned char j = 0; j < channels; j++) {
    for (i = 1; i < dataSize; i++) {
      dataOut[j][i] = dataIn[j][i] - dataIn[j][i-1];
    }
  }
  i = 0;
}

void USCAudioProcess::integrate() {
  dataOut[0] = dataIn[0];
  for (unsigned char j = 0; j < channels; j++) {
    for (i = 1; i < dataSize; i++) {
      dataOut[j][i] = dataOut[j][i-1] + dataIn[j][i];
    }
  }
  i = 0;
}
