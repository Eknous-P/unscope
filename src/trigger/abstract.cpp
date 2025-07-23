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

#include "trigger.h"

void Trigger::setupTrigger(unscopeParams* up, float* cb) {
  uParams  = up;
  alignBuf = NULL;
  chanBuf  = cb;

  params = {

  };

  triggered = false;
  alignRegionSize = 0;
}

vector<TriggerParam> Trigger::getParams() {
  return params;
}

bool Trigger::trigger(nint windowSize) {
  return false;
}

bool Trigger::getTriggered() {
  return triggered;
}

float* Trigger::getAlignBuffer() {
  return alignBuf;
}

nint Trigger::getAlignRegionSize() {
  return alignRegionSize;
}

Trigger::~Trigger(){
  for (TriggerParam i:params) i.destroy();
}
