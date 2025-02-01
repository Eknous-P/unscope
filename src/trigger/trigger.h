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
          *(bool*)valuePtr = false; \
          break; \
        case TP_KNOBNORM: \
          valuePtr = new float; \
          *(float*)valuePtr = 0.0f; \
          break; \
        default: \
          valuePtr = new int; \
          *(int*)valuePtr = 0; \
          break; \
      }

enum TriggerParamTypes : unsigned char {
  TP_NONE = 0,
  TP_TOGGLE,
  TP_KNOBNORM, // [-1,1]
};

const unsigned char TriggerParamTypeSize[]={
  sizeof(int),
  sizeof(bool),
  sizeof(float)
};

class TriggerParam {
  TriggerParamTypes type;
  bool exactInput;
  void *valuePtr;
  const char *label, *desc;
  public:
    bool bindToDragX, bindToDragY;
    void draw() {
      switch (type) {
        case TP_TOGGLE: {
          ImGui::Toggle(label, (bool*)valuePtr);
          break;
        }
        case TP_KNOBNORM: {
          ImGuiKnobs::Knob(label, (float*)valuePtr, -1.0f, 1.0f, 0.0f, "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
          if (exactInput) RIGHTCLICK_EXACT_INPUT((float*)valuePtr, ImGuiDataType_Float, );
          if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) *(float*)valuePtr = 0.0f;
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

    void *getValue() {
      return valuePtr;
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

    TriggerParam(TriggerParamTypes t, bool i, const char* l, bool dragX, bool dragY):
      desc(NULL) {
      type       = t;
      exactInput = i;
      label      = l;
      bindToDragX = dragX;
      bindToDragY = dragY;
      INIT_PARAM_VALUE
    }

    TriggerParam(TriggerParamTypes t, bool i, const char* l, const char* d, bool dragX, bool dragY) {
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
          if (valuePtr) delete (float*)valuePtr;
          break;
        default:
          if (valuePtr) delete (int*)valuePtr;
          break;
      }
    }
};

class Trigger {
  float *alignBuf, *chanBuf;
  unscopeParams* uParams;
  std::vector<TriggerParam> params;

  bool triggered;

  public:
    virtual void setupTrigger(unscopeParams* up, float* cb);
    virtual std::vector<TriggerParam> getParams();
    virtual bool trigger(unsigned long int windowSize);
    virtual bool getTriggered();
    virtual float* getAlignBuffer();
    virtual ~Trigger();
};

enum Triggers {
  TRIG_NONE = 0,
  TRIG_FALLBACK,
  TRIG_ANALOG,
  TRIG_MAX
};

#endif
