/*
unscope - an audio oscilloscope
Copyright (C) 2025-2026 Eknous

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

void TriggerAnalog::setupTrigger(DataBuffer* buf) {
  buffer = buf;

  params = {
    TriggerParam(PARAM_KNOBNORM,false,"level",NULL,false,true),
    TriggerParam(PARAM_TOGGLE,false,"extend trigger range","allows trigger to scan for the full audio buffer,\ninstead of the visible range",NULL,false,false),
    TriggerParam(PARAM_TOGGLE,false,"trigger edge","off - rising\non - falling",NULL,false,false),
  };

  triggerIndex = 0;

  triggered = true;
}

bool TriggerAnalog::trigger(nint windowSize) {
  triggered = false;
  // locate trigger
  bool triggerHigh = false, triggerLow = false, foundTrigger = false, edge = !params[2].getValue<bool>();
  float trigY = params[0].getValue<float>(); // temp
  triggerIndex = buffer->getSize() - windowSize;

  while (triggerIndex > 0) {
    triggerIndex--;
    if (buffer->getValueScaled(triggerIndex + windowSize/2) < trigY) {
      triggerLow = true;
      CHECK_TRIGGERED;
      if (foundTrigger && edge) {
        triggered = true;
        return true;
      }
    }
    if (buffer->getValueScaled(triggerIndex + windowSize/2) > trigY) {
      triggerHigh = true;
      CHECK_TRIGGERED;
      if (foundTrigger && !edge) {
        triggered = true;
        return true;
      }
    }
    if (!params[1].getValue<bool>()) {
      if (triggerIndex < buffer->getSize() - 2 * windowSize) return false; // out of window
    }
  }
  return false;
}

TriggerAnalog::~TriggerAnalog() {
  for (TriggerParam i:params) i.destroy();
}
