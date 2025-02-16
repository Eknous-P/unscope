#include "trigger.h"

#ifndef TRIGGER_ANALOG_H
#define TRIGGER_ANALOG_H

class TriggerAnalog : public Trigger {
  float *alignBuf, *chanBuf;
  unsigned long int bufferSize;
  std::vector<TriggerParam> params;

  long int triggerIndex;

  bool triggered;

  public:
    void setupTrigger(unsigned long int bs, float* cb);
    std::vector<TriggerParam> getParams();
    bool trigger(unsigned long int windowSize);
    bool getTriggered();
    float* getAlignBuffer();
    ~TriggerAnalog();
};

#endif
