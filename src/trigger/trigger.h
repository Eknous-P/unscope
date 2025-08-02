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
#include "imgui.h"
#include "imgui-knobs.h"
#include "imgui_toggle.h"

#define INIT_PARAM_VALUE \
      switch (type) { \
        case TP_TOGGLE: \
          valuePtr = new bool; \
          break; \
        case TP_KNOBNORM: \
        case TP_KNOBUNIT: \
          valuePtr = new float; \
          break; \
        default: \
          valuePtr = new int; \
          break; \
      } \
      memset(valuePtr, 0, TriggerParamTypeSize[type]);

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
  bool exactInput;
  void *valuePtr;
  const char *label, *desc;
  public:
    bool bindToDragX, bindToDragY;

    const char* getLabel() {
      return label;
    }

    void *getValuePtr() {
      return valuePtr;
    }

    template<typename T>
    T getValue() {
      return *(T*)valuePtr;
    }

    template<typename T>
    void setValue(T v) {
      *(T*)valuePtr = v;
    }

    void draw() {
      switch (type) {
        case TP_TOGGLE: {
          ImGui::Toggle(label, (bool*)valuePtr);
          break;
        }
        case TP_KNOBNORM: {
          ImGuiKnobs::Knob(label, (float*)valuePtr, -1.0f, 1.0f, 0.0f, "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
          if (exactInput) RIGHTCLICK_EXACT_INPUT((float*)valuePtr, ImGuiDataType_Float, {
            if (getValue<float>() > 1.0f) setValue<float>(1.0f);
            if (getValue<float>() < -1.0f) setValue<float>(-1.0f);
          });
          if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) setValue<float>(0.0f);
          break;
        }
        case TP_KNOBUNIT: {
          ImGuiKnobs::Knob(label, (float*)valuePtr, 0.0f, 1.0f, 0.0f, "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
          if (exactInput) RIGHTCLICK_EXACT_INPUT((float*)valuePtr, ImGuiDataType_Float, {
            if (getValue<float>() > 1.0f) setValue<float>(1.0f);
            if (getValue<float>() < 0.0f) setValue<float>(0.0f);
          });
          if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) setValue<float>(0.0f);
          break;
        }
        default: break;
      }
      if (desc) {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
          ImGui::SetTooltip("%s", desc);
        }
      }
    }

    TriggerParamTypes getType() {
      return type;
    }

    TriggerParam():
      type(TP_NONE),
      exactInput(false),
      valuePtr(NULL),
      label(NULL),
      desc(NULL) {}

    TriggerParam(TriggerParamTypes t, bool i, const char* l, bool dragX=false, bool dragY=false):
      desc(NULL) {
      type       = t;
      exactInput = i;
      label      = l;
      bindToDragX = dragX;
      bindToDragY = dragY;
      INIT_PARAM_VALUE
    }

    TriggerParam(TriggerParamTypes t, bool i, const char* l, const char* d, bool dragX=false, bool dragY=false) {
      type       = t;
      exactInput = i;
      label      = l;
      desc       = d;
      bindToDragX = dragX;
      bindToDragY = dragY;
      INIT_PARAM_VALUE
    }

    void destroy() {
      switch (type) {
        case TP_TOGGLE:
          if (valuePtr) delete (bool*)valuePtr;
          break;
        case TP_KNOBNORM:
        case TP_KNOBUNIT:
          if (valuePtr) delete (float*)valuePtr;
          break;
        default:
          if (valuePtr) delete (int*)valuePtr;
          break;
      }
    }
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
