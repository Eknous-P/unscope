#include "fallback.h"
#include <shared.h>

void TriggerFallback::setupTrigger(unscopeParams* up, float** cb) {
  uParams  = up;
  chanBuf  = cb;

  alignBuf = new float*[up->channels];
  if (alignBuf) {
    for (unsigned char i = 0; i < up->channels; i++) {
      alignBuf[i] = new float[up->audioBufferSize];
    }
  }

  oldWindowSize = 0;

  params = {

  };

  triggered = true;
}

void TriggerFallback::drawParams() {
  for (TriggerParam i:params) i.draw();
}

void TriggerFallback::trigger(unsigned char chan, unsigned long int windowSize) {
  // theres no need to run this loop every time once its set,
  // since it doesnt change unless the window size does
  if (oldWindowSize==windowSize) return;

  oldWindowSize = windowSize;
  const float delta = 2.0f / windowSize;

  // -1.498.... close enough to -1
  memset(alignBuf[chan], 0xbf, (uParams->audioBufferSize - 2) * sizeof(float));

  alignBuf[chan][uParams->audioBufferSize-1] = 1.0f;

  for (unsigned long int i = uParams->audioBufferSize-2; i > 0; i--) {
    float v = alignBuf[chan][i+1] - delta;
    if (v < -1.0f) break;
    alignBuf[chan][i] = v;
  }
}

float** TriggerFallback::getAlignBuffer() {
  return alignBuf;
}

bool TriggerFallback::getTriggered() {
  return triggered;
}

TriggerFallback::~TriggerFallback() {
  if (alignBuf) {
    for (unsigned char z = 0; z < uParams->channels; z++) {
      if (alignBuf[z]) {
        delete[] alignBuf[z];
        alignBuf[z] = __null;
      }
    }
    delete[] alignBuf;
    alignBuf = __null;
  }
}