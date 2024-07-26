#include "../audio.h"

void ProcessNode::attachTo(float* in, unsigned char _input) {
  inputs[_input] = in;
}

float* ProcessNode::getOutput() {
  return output;
}

const char* ProcessNode::getName() {
  return def.name;
}

void ProcessNode::process() {
  
}