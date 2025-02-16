#include "analog.h"

#define CHECK_TRIGGERED foundTrigger=triggerLow&&triggerHigh

void TriggerAnalog::setupTrigger(unsigned long int bs, float* cb) {
  chanBuf = cb;
  bufferSize = bs;

  alignBuf = new float[bufferSize];

  params = {
    TriggerParam(TP_KNOBNORM,false,"level",false,true),
    TriggerParam(TP_KNOBNORM,false,"x offset",true,false),
    TriggerParam(TP_TOGGLE,false,"extend trigger range","allows trigger to scan for the full audio buffer,\ninstead of the visible range",false,false),
    TriggerParam(TP_TOGGLE,false,"trigger edge","off - rising\non - falling",false,false),
  };

  triggerIndex = 0;

  triggered = true;
}

std::vector<TriggerParam> TriggerAnalog::getParams() {
  return params;
}

bool TriggerAnalog::trigger(unsigned long int windowSize) {
  triggered = false;
  // locate trigger
  bool triggerHigh = false, triggerLow = false, foundTrigger = false, edge = !*(bool*)params[3].getValue();
  float trigY = *(float*)params[0].getValue(); // temp
  long int offset = (windowSize / 2) * *(float*)params[1].getValue();
  // is this better than multiplying an int with x.5f?
  long int windowSizeHalf = windowSize / 2;
  triggerIndex = bufferSize - windowSize - windowSizeHalf;
  // -1 offset will reveal the oob area of the buffer
  // so -windowSizeHalf

  memset(alignBuf, 0xbf, bufferSize * sizeof(float));

  while (triggerIndex > 0) {
    triggerIndex--;
    if (chanBuf[triggerIndex + windowSize/2] < trigY) {
      triggerLow = true;
      CHECK_TRIGGERED;
      if (foundTrigger && edge) break;
    }
    if (chanBuf[triggerIndex + windowSize/2] > trigY) {
      triggerHigh = true;
      CHECK_TRIGGERED;
      if (foundTrigger && !edge) break;
    }
    if (!*(bool*)params[2].getValue()) {
      if (triggerIndex < bufferSize - 2 * windowSize - windowSizeHalf) return false; // out of window
    }
  }

  triggerIndex-=offset;

  alignBuf[triggerIndex] = -1.0f;

  const float delta = 2.0f / windowSize;
  unsigned long int i = triggerIndex + 1;
  for (; i < bufferSize; i++) {
    float v = alignBuf[i-1] + delta;
    if (v > 1.0f) break;
    alignBuf[i] = v;
  }

  // 3.003...... not quite 1...
  memset(&alignBuf[i], 0x40, (bufferSize-i)*sizeof(float));

  triggered = true;
  return true;
}

float* TriggerAnalog::getAlignBuffer() {
  return alignBuf;
}

bool TriggerAnalog::getTriggered() {
  return triggered;
}

TriggerAnalog::~TriggerAnalog() {
  for (TriggerParam i:params) i.destroy();
  DELETE_PTR(alignBuf)
}