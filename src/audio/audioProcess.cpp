#include "audio.h"
#include "processNodes/blank.h"

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

  nodes.clear();
  for (unsigned short i = 0; i < 256; i++) {
    nodes.push_back(NULL);
  }
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

ProcessNode* USCAudioProcess::addNode(ProcessNodes nodeId) {
  ProcessNode* newNode = NULL;
  switch (nodeId) {
    case PNODE_BLANK:
      newNode = new PNode_Blank;
      nodes.push_back(newNode);
      break;
    default: break;
  }
  return newNode;
}

ProcessNodeParam PNPInit(const char* n, float v, float mn, float mx) {
  ProcessNodeParam ass;
  ass.name = n;
  ass.value = v;
  ass.vMin = mn;
  ass.vMax = mx;
  return ass;
}
