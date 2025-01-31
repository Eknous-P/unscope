#include "fallback.h"

void TriggerFallback::setupTrigger(unscopeParams* up, float* cb) {
  uParams = up;
  chanBuf = cb;

  alignBuf = new float[up->audioBufferSize];

  oldWindowSize = 0;

  params = {

  };

  triggered = true;
}

std::vector<TriggerParam> TriggerFallback::getParams() {
  return params;
}

bool TriggerFallback::trigger(unsigned long int windowSize) {
  // theres no need to run this loop every time once its set,
  // since it doesnt change unless the window size does
  if (oldWindowSize==windowSize) return true;

  oldWindowSize = windowSize;
  const float delta = 2.0f / windowSize;

  // -1.498.... close enough to -1
  memset(alignBuf, 0xbf, (uParams->audioBufferSize - 2) * sizeof(float));

  alignBuf[uParams->audioBufferSize-1] = 1.0f;

  for (unsigned long int i = uParams->audioBufferSize-2; i > 0; i--) {
    float v = alignBuf[i+1] - delta;
    if (v < -1.0f) break;
    alignBuf[i] = v;
  }

  return true;
}

float* TriggerFallback::getAlignBuffer() {
  return alignBuf;
}

bool TriggerFallback::getTriggered() {
  return triggered;
}

TriggerFallback::~TriggerFallback() {
  for (TriggerParam i:params) i.destroy();
  DELETE_PTR(alignBuf)
}