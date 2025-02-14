#include "trigger.h"

void Trigger::setupTrigger(unsigned long int bs, float* cb) {
  alignBuf   = NULL;
  chanBuf    = cb;
  bufferSize = bs;

  params = {

  };

  triggered = false;
}

std::vector<TriggerParam> Trigger::getParams() {
  return params;
}

bool Trigger::trigger(unsigned long int windowSize) {
  return false;
}

bool Trigger::getTriggered() {
  return triggered;
}

float* Trigger::getAlignBuffer() {
  return alignBuf;
}

Trigger::~Trigger(){
  for (TriggerParam i:params) i.destroy();
}