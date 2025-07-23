/*
unscope - an audio oscilloscope
Copyright (C) 2025 Eknous

unscope is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

unscope is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
unscope. If not, see <https://www.gnu.org/licenses/>. 
*/

#include "fallback.h"

void TriggerFallback::setupTrigger(unscopeParams* up, float* cb) {
  uParams = up;
  chanBuf = cb;

  alignBuf = new float[up->audioBufferSize];

  params = {

  };

  triggered = true;
  alignRegionSize = 0;
}

vector<TriggerParam> TriggerFallback::getParams() {
  return params;
}

bool TriggerFallback::trigger(nint windowSize) {
  // theres no need to run this loop every time once its set,
  // since it doesnt change unless the window size does
  if (alignRegionSize==windowSize) return true;

  alignRegionSize = windowSize;
  const float delta = 2.0f / windowSize;

  // -1.498.... close enough to -1
  memset(alignBuf, 0xbf, (uParams->audioBufferSize - 2) * sizeof(float));

  alignBuf[uParams->audioBufferSize-1] = 1.0f;

  for (nint i = uParams->audioBufferSize-2; i > 0; i--) {
    float v = alignBuf[i+1] - delta;
    if (v < -1.0f) break;
    alignBuf[i] = v;
  }

  return true;
}

float* TriggerFallback::getAlignBuffer() {
  return alignBuf;
}

nint TriggerFallback::getAlignRegionSize() {
  return alignRegionSize;
}

bool TriggerFallback::getTriggered() {
  return triggered;
}

TriggerFallback::~TriggerFallback() {
  for (TriggerParam i:params) i.destroy();
  DELETE_PTR_ARR(alignBuf)
}
