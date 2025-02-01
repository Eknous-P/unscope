#include "analog.h"

#define CHECK_TRIGGERED foundTrigger=triggerLow&&triggerHigh

void TriggerAnalog::setupTrigger(unscopeParams* up, float* cb) {
  uParams = up;
  chanBuf = cb;

  alignBuf = new float[up->audioBufferSize];

  params = {
    TriggerParam(TP_KNOBNORM,false,"level",false,true),
    TriggerParam(TP_KNOBNORM,true,"x offset",true,false),
    TriggerParam(TP_TOGGLE,false,"extend trigger range","allows trigger to scan for the full audio buffer,\ninstead of the visible range",false,false),
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
  bool triggerHigh = false, triggerLow = false, foundTrigger = false;
  float trigY = *(float*)params[0].getValue(); // temp
  long int offset = (windowSize / 2) * *(float*)params[1].getValue();
  triggerIndex = uParams->audioBufferSize - windowSize;

  memset(alignBuf, 0xbf, uParams->audioBufferSize * sizeof(float));

  while (triggerIndex > 0) {
    triggerIndex--;
    if (chanBuf[triggerIndex + windowSize/2 + offset] < trigY) {
      triggerLow = true;
      CHECK_TRIGGERED;
      if (foundTrigger && true) break;
    }
    if (chanBuf[triggerIndex + windowSize/2 + offset] > trigY) {
      triggerHigh = true;
      CHECK_TRIGGERED;
      if (foundTrigger && !true) break;
    }
    if (!*(bool*)params[2].getValue()) {
      if (triggerIndex < uParams->audioBufferSize - 2 * windowSize) return false; // out of window
    }
  }

  alignBuf[triggerIndex] = -1.0f;

  const float delta = 2.0f / windowSize;
  unsigned long int i = triggerIndex + 1;
  for (; i < uParams->audioBufferSize; i++) {
    float v = alignBuf[i-1] + delta;
    if (v > 1.0f) break;
    alignBuf[i] = v;
  }

  // 3.003...... not quite 1...
  memset(&alignBuf[i], 0x40, (uParams->audioBufferSize-i)*sizeof(float));

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