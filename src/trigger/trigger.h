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

#ifndef TRIGGER_H
#define TRIGGER_H

#include "shared.h"

enum TriggerParamTypes : unsigned char {
  TP_NONE = 0,
  TP_TOGGLE,
  TP_KNOBNORM, // [-1,1]
  TP_KNOBUNIT, // [0,1]
};

const unsigned char TriggerParamTypeSize[]={
  sizeof(int),
  sizeof(bool),
  sizeof(float),
  sizeof(float)
};

class TriggerParam {
  TriggerParamTypes type;
  void *valuePtr;
  const char *label, *desc;
  bool exactInput;
  bool hovered, active;
  public:
    bool bindToDragX, bindToDragY;

    const char* getLabel();
    void *getValuePtr();

    template<typename T>
    T getValue() {return *(T*)valuePtr;}

    template<typename T>
    void setValue(T v) {*(T*)valuePtr = v;}

    bool draw();
    TriggerParamTypes getType();
    bool isHovered();
    bool isActive();

    TriggerParam();
    TriggerParam(TriggerParamTypes t, bool i, const char* l, bool dragX=false, bool dragY=false);
    TriggerParam(TriggerParamTypes t, bool i, const char* l, const char* d, bool dragX=false, bool dragY=false);
    void destroy();
};

class Trigger {
  float *chanBuf;
  unscopeParams* uParams;
  vector<TriggerParam> params;

  nint triggerIndex;

  bool triggered;
  nint alignRegionSize;

  public:
    virtual void setupTrigger(unscopeParams* up, float* cb);
    virtual vector<TriggerParam> getParams();
    virtual bool trigger(nint windowSize);
    virtual bool getTriggered();
    virtual nint getAlignRegionSize();
    virtual nint getTriggerIndex();
    virtual ~Trigger();
};

enum Triggers {
  TRIG_NONE = 0,
  TRIG_FALLBACK,
  TRIG_ANALOG,
  TRIG_SMOOTH,
  TRIG_MAX
};

#endif
