#include "trigger.h"

#ifndef TRIGGER_FALLBACK_H
#define TRIGGER_FALLBACK_H

class TriggerFallback : public Trigger {
  float *alignBuf, *chanBuf;
  unscopeParams* uParams;
  std::vector<TriggerParam> params;
  unsigned long int oldWindowSize;

  bool triggered;

  public:
    void setupTrigger(unscopeParams* up, float* cb);
    std::vector<TriggerParam> getParams();
    bool trigger(unsigned long int windowSize);
    bool getTriggered();
    float* getAlignBuffer();
    ~TriggerFallback();
};

#endif
