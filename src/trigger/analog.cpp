#include "analog.h"
#include <shared.h>
#include <trigger.h>

#define CHECK_TRIGGERED foundTrigger=triggerLow&&triggerHigh

void TriggerAnalog::setupTrigger(unscopeParams* up, float* cb) {
  uParams = up;
  chanBuf = cb;

  alignBuf = new float[up->audioBufferSize];

  params = {
    TriggerParam(TP_KNOBNORM,false,"level"),
  };

  triggerIndex = 0;

  triggered = true;
}

void TriggerAnalog::drawParams() {
  for (TriggerParam i:params) i.draw();
}

bool TriggerAnalog::trigger(unsigned long int windowSize) {
  triggered = false;
  // locate trigger
  bool triggerHigh = false, triggerLow = false, foundTrigger = false;
  float trigY = *(float*)params[0].getValue(); // temp
  triggerIndex = uParams->audioBufferSize-windowSize;

  memset(alignBuf, 0xbf, uParams->audioBufferSize * sizeof(float));

  while (triggerIndex > 0) {
    triggerIndex--;
    if (chanBuf[triggerIndex + windowSize/2] < trigY) {
      triggerLow = true;
      CHECK_TRIGGERED;
      if (foundTrigger && true) break;
    }
    if (chanBuf[triggerIndex + windowSize/2] > trigY) {
      triggerHigh = true;
      CHECK_TRIGGERED;
      if (foundTrigger && !true) break;
    }
    if (triggerIndex < uParams->audioBufferSize - 2 * windowSize) return false; // out of window
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
  DELETE_PTR(alignBuf)
}