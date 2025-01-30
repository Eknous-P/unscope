#include "trigger.h"

void Trigger::setupTrigger(unscopeParams* up, float* cb) {
  uParams  = up;
  alignBuf = NULL;
  chanBuf  = cb;

  params = {

  };

  triggered = false;
}

void Trigger::drawParams() {
  for (TriggerParam i:params) i.draw();
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
}