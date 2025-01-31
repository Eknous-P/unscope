#ifndef TRIGGER_H
#define TRIGGER_H

#include "shared.h"
#include "imgui.h"
#include "imgui-knobs.h"

enum TriggerParamTypes : unsigned char {
  TP_NONE = 0,
  TP_BOOL,
  TP_KNOBNORM, // [-1,1]
  TP_CHAN,
};

class TriggerParam {
  TriggerParamTypes type;
  bool exactInput;
  void *valuePtr;
  const char* label;
  public:
    void draw() {
      switch (type) {
        case TP_BOOL: {
          ImGui::Checkbox(label, (bool*)valuePtr);
          break;
        }
        case TP_KNOBNORM: {
          ImGuiKnobs::Knob(label, (float*)valuePtr, -1.0f, 1.0f, 0.0f, "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
          if (exactInput) RIGHTCLICK_EXACT_INPUT(label, *(float*)valuePtr, );
          if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) *(float*)valuePtr = 0.0f;
          break;
        }
        default: break;
      }
    }

    void *getValue() {
      return valuePtr;
    }

    TriggerParam():
      type(TP_NONE),
      exactInput(false),
      valuePtr(NULL),
      label(NULL) {}

    TriggerParam(TriggerParamTypes t, bool i, const char* l) {
      type       = t;
      exactInput = i;
      label      = l;

      switch (type) {
        case TP_BOOL:
          valuePtr = new bool;
          *(bool*)valuePtr = false;
          break;
        case TP_KNOBNORM:
          valuePtr = new float;
          *(float*)valuePtr = 0.0f;
          break;
        default:
          valuePtr = new int;
          *(int*)valuePtr = 0;
          break;
      }
    }
    void destroy() {
      switch (type) {
        case TP_BOOL:
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
    virtual void drawParams();
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
