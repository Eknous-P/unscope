#include "blank.h"

PNode_Blank::PNode_Blank() {
  inputs[0] = NULL;
  inputs[1] = NULL;
  inputs[2] = NULL;
  inputs[3] = NULL;

  output = 0;

  // param setup
  params[0] = PNPInit(NULL,0,0,0);
  params[1] = PNPInit(NULL,0,0,0);
  params[2] = PNPInit(NULL,0,0,0);
  params[3] = PNPInit(NULL,0,0,0);
}

ProcessNodeDefines PNode_Blank::getDefines() {
  return ProcessNodeDefines("Blank",1,0);
}

void PNode_Blank::process() {
  *output = *inputs[0];
}