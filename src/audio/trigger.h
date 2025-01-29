#ifndef TRIGGER_H
#define TRIGGER_H

#include "shared.h"

enum TriggerParamTypes : unsigned char {
  TP_NONE = 0,
  TP_BOOL,
  TP_KNOBNORM, // [-1,1]
  TP_CHAN,
};

struct TriggerParam {
  TriggerParamTypes type;
  bool exactInput;
  void *valuePtr;
  const char* label;

  void draw() {}

  TriggerParam():
    type(TP_NONE),
    exactInput(false),
    valuePtr(NULL),
    label(NULL) {}

  TriggerParam(TriggerParamTypes t, bool i, void *p, const char* l) {
    type       = t;
    exactInput = i;
    valuePtr   = p;
    label      = l;
  }
};

class Trigger {
  float **alignBuf, **chanBuf;
  unscopeParams* uParams;
  std::vector<TriggerParam> params;

  bool triggered;

  public:
    virtual void setupTrigger(unscopeParams* up, float** cb);
    virtual void drawParams();
    virtual bool trigger(unsigned char chan, unsigned long int windowSize);
    virtual bool getTriggered();
    virtual float** getAlignBuffer();
    virtual ~Trigger();
};

enum Triggers {
  TRIG_NONE = 0,
  TRIG_FALLBACK,
  TRIG_ANALOG,
  TRIG_MAX
};

#endif
