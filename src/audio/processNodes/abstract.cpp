#include "../audio.h"

void ProcessNode::attachTo(float* in, unsigned char _input) {
  inputs[_input] = in;
}

float* ProcessNode::getOutput() {
  return output;
}

ProcessNodeDefines ProcessNode::getDefines() {
  return ProcessNodeDefines(NULL,0,0);
}

void ProcessNode::process() {

}