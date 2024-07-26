#ifndef PNODE_BLANK_H
#define PNODE_BLANK_H

#include "../audio.h"

class PNode_Blank : public ProcessNode {
  float *inputs[4], *output;
  public:
    ProcessNodeParam params[4];
    void attachTo(float* in, unsigned char _input);
    float* getOutput();
    ProcessNodeDefines getDefines();
    void process();

    PNode_Blank();
    ~PNode_Blank();
};

#endif