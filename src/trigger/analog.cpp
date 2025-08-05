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

#include "analog.h"

#define CHECK_TRIGGERED foundTrigger=triggerLow&&triggerHigh

void TriggerAnalog::setupTrigger(unscopeParams* up, float* cb) {
  uParams = up;
  chanBuf = cb;

  params = {
    TriggerParam(TP_KNOBNORM,false,"level",false,true),
    TriggerParam(TP_TOGGLE,false,"extend trigger range","allows trigger to scan for the full audio buffer,\ninstead of the visible range",false,false),
    TriggerParam(TP_TOGGLE,false,"trigger edge","off - rising\non - falling",false,false),
  };

  triggerIndex = 0;

  triggered = true;
  alignRegionSize = 0;
}

vector<TriggerParam> TriggerAnalog::getParams() {
  return params;
}

bool TriggerAnalog::trigger(nint windowSize) {
  if (!chanBuf) return false;
  triggered = false;
  // locate trigger
  bool triggerHigh = false, triggerLow = false, foundTrigger = false, edge = !params[2].getValue<bool>();
  float trigY = params[0].getValue<float>(); // temp
  triggerIndex = uParams->audioBufferSize - windowSize;

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
    if (!params[1].getValue<bool>()) {
      if (triggerIndex < uParams->audioBufferSize - 2 * windowSize) return false; // out of window
    }
  }


  if (triggerIndex > uParams->audioBufferSize - windowSize) {
    alignRegionSize = uParams->audioBufferSize - triggerIndex;
  } else {
    alignRegionSize = windowSize;
  }

  triggered = true;
  return true;
}

nint TriggerAnalog::getAlignRegionSize() {
  return alignRegionSize;
}

#ifdef PROGRAM_DEBUG
nint TriggerAnalog::getTriggerIndex() {
  return triggerIndex;
}
#endif

bool TriggerAnalog::getTriggered() {
  return triggered;
}

TriggerAnalog::~TriggerAnalog() {
  for (TriggerParam i:params) i.destroy();
}
