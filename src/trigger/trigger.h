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

#ifndef TRIGGER_H
#define TRIGGER_H

#include "shared.h"
#include "param.h"
#include "data.h"

class TriggerParam : public Parameter {
  public:
    bool bindToDragX, bindToDragY;
    TriggerParam(ParamTypes t, bool i, const char* n, const char* l, const char* d=NULL, void* ext=NULL, void* val=NULL, bool dragX=false, bool dragY=false):
      Parameter(t, i, n, l, d, ext, val),
      bindToDragX(dragX),
      bindToDragY(dragY) {}
};

class Trigger {
  protected:
    DataBuffer* buffer;
    vector<TriggerParam> params;

    nint triggerIndex;

    bool triggered;
  public:
    virtual void setupTrigger(DataBuffer* buf);
    virtual vector<TriggerParam>* getParams();
    virtual bool trigger(nint windowSize);
    virtual bool getTriggered();
    virtual nint getTriggerIndex();
    virtual ~Trigger();
};

enum Triggers : signed char {
  TRIG_INVALID  = -1,
  TRIG_FALLBACK = 0,
  TRIG_ANALOG,
  TRIG_SMOOTH,
  TRIG_MAX,
  TRIG_STOLEN
};

#endif
