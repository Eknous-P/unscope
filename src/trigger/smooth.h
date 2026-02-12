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

#ifndef TRIGGER_SMOOTH_H
#define TRIGGER_SMOOTH_H

#include "trigger.h"

class TriggerSmooth : public Trigger {
  float* smoothBuf, triggerLevel;

  float prevSmooth, logSmooth;

  public:
    void setupTrigger(DataBuffer* buf);
    bool trigger(nint windowSize);

    float* getSmoothBuffer();
    float getTriggerLevel();
    ~TriggerSmooth();
};

#endif
