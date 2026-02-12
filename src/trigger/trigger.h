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
    TriggerParam(ParamTypes t, bool i, const char* l, void* ext=NULL, bool dragX=false, bool dragY=false) {
      desc       = NULL;
      hovered    = false;
      active     = false;
      type       = t;
      exactInput = i;
      label      = l;
      paramData  = ext;
      bindToDragX = dragX;
      bindToDragY = dragY;
      INIT_PARAM_VALUE
    }
    TriggerParam(ParamTypes t, bool i, const char* l, const char* d, void* ext=NULL, bool dragX=false, bool dragY=false) {
      hovered    = false;
      active     = false;
      type       = t;
      exactInput = i;
      label      = l;
      desc       = d;
      paramData  = ext;
      bindToDragX = dragX;
      bindToDragY = dragY;
      INIT_PARAM_VALUE
    }
};

class Trigger {
  protected:
    DataBuffer* buffer;
    vector<TriggerParam> params;

    nint triggerIndex;

    bool triggered;
  public:
    virtual void setupTrigger(DataBuffer* buf);
    virtual vector<TriggerParam> getParams();
    virtual bool trigger(nint windowSize);
    virtual bool getTriggered();
    virtual nint getTriggerIndex();
    virtual ~Trigger();
};

enum Triggers : int {
  TRIG_INVALID  = -1,
  TRIG_FALLBACK = 0,
  TRIG_ANALOG,
  TRIG_SMOOTH,
  TRIG_MAX
};

#endif
