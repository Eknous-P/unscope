/*
unscope - an audio oscilloscope
Copyright (C) 2025-2026 Eknous

unscope is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

unscope is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
unscope. If not, see <https://www.gnu.org/licenses/>. 
*/

#include "dummy.h"
#include <data.h>

constexpr float freqKnobLimits[2]={0,8000};

int DummyDriver::callbackFunction(void* self, nint samples) {
  DummyDriver* selfPtr=(DummyDriver*)self;
  for (nint i=0; i<samples; i++) {
    float sample=sin(selfPtr->index++*M_2_PI*selfPtr->config[1].getValue<float>())*selfPtr->config[0].getValue<float>();
    selfPtr->getBuffer(0)->write(&sample);
  }
  return 0;
}

const DataDriverInfo DummyDriver::getDriverInfo() const {
  return {
    DATA_DUMMY,
    DRIVERFLAG_OUTPUT,
    "Dummy Driver"
  };
}

int DummyDriver::setup(USCData* p) {
  parent       = p;
  state        = DRIVERSTATE_IDLE;
  config = {
    Parameter(PARAM_KNOBUNIT, false, "dummyAmpl", "amplitude"),
    Parameter(PARAM_INPUTFLOAT, false, "dummyFreq", "frequency", NULL, (float*)freqKnobLimits),
  };
  buffers.push_back(new DataBuffer_Float);
  buffers[0]->init(65536, 48000, "dummy sine");
  return 0;
}
