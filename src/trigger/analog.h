#include "trigger.h"

#ifndef TRIGGER_ANALOG_H
#define TRIGGER_ANALOG_H

class TriggerAnalog : public Trigger {
  float **alignBuf, **chanBuf;
  unscopeParams* uParams;
  std::vector<TriggerParam> params;

  unsigned long int triggerIndex;

  bool triggered;

  public:
    void setupTrigger(unscopeParams* up, float** cb);
    void drawParams();
    bool trigger(unsigned char chan, unsigned long int windowSize);
    bool getTriggered();
    float** getAlignBuffer();
    ~TriggerAnalog();
};

#endif
