#include "trigger.h"

#ifndef TRIGGER_ANALOG_H
#define TRIGGER_ANALOG_H

class TriggerAnalog : public Trigger {
  float *alignBuf, *chanBuf;
  unscopeParams* uParams;
  std::vector<TriggerParam> params;

  unsigned long int triggerIndex;

  bool triggered;

  public:
    void setupTrigger(unscopeParams* up, float* cb);
    std::vector<TriggerParam> getParams();
    bool trigger(unsigned long int windowSize);
    bool getTriggered();
    float* getAlignBuffer();
    ~TriggerAnalog();
};

#endif
