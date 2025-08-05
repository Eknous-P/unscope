/*
unscope - an audio oscilloscope
Copyright (C) 2025 Eknous

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

#include "smooth.h"

#define CHECK_TRIGGERED foundTrigger=triggerLow&&triggerHigh
#define LOG_F_LEVEL 100.0f
#define LOG_FUNC(x) log(x*LOG_F_LEVEL+1.f)/log(LOG_F_LEVEL+1.f)

void TriggerSmooth::setupTrigger(unscopeParams* up, float* cb) {
  uParams = up;
  chanBuf = cb;

  smoothBuf = new float[up->audioBufferSize];

  triggerLevel = 0.0f;

  params = {
    TriggerParam(TP_KNOBUNIT,false,"smoothing"),
    TriggerParam(TP_KNOBUNIT,false,"level ratio"),
    TriggerParam(TP_TOGGLE,false,"trigger on minimum"),
  };

  params[0].setValue<float>(.9f);
  params[1].setValue<float>(.3f);

  triggerIndex = 0;

  prevSmooth = 0.f;
  logSmooth  = 0.f;

  triggered = true;
  alignRegionSize = 0;
}

vector<TriggerParam> TriggerSmooth::getParams() {
  return params;
}

bool TriggerSmooth::trigger(nint windowSize) {
  int iter = 0;
  triggered = false;
  int PRECALC_RANGE = windowSize;
  nint begin = uParams->audioBufferSize - windowSize;
  const bool useMin = params[2].getValue<bool>();
  if (begin < PRECALC_RANGE) PRECALC_RANGE = 0;
  float smoothLvl = params[0].getValue<float>();
  if (prevSmooth != smoothLvl) {
    prevSmooth = smoothLvl;
    logSmooth = LOG_FUNC(prevSmooth);
  }
  // smooth waveform
  memset(smoothBuf, 0xff, (begin-PRECALC_RANGE)*sizeof(float));
  smoothBuf[begin - PRECALC_RANGE] = chanBuf[begin - PRECALC_RANGE] * (1.0f - logSmooth);
  float smoothPeak = useMin?1.0f:-1.0f;
  for (nint i = begin + 1 - PRECALC_RANGE; i < uParams->audioBufferSize; i++) {
    smoothBuf[i] = (logSmooth * smoothBuf[i-1] + (1.0f - logSmooth) * chanBuf[i]);
    if (i < begin) continue;
    if (useMin) {
      if (smoothBuf[i] < smoothPeak) {
        smoothPeak = smoothBuf[i];
      }
    } else {
      if (smoothBuf[i] > smoothPeak) {
        smoothPeak = smoothBuf[i];
      }
    }
  }
  while (iter--) {
    smoothPeak = useMin?1.0f:-1.0f;
    for (nint i = begin + 1 - PRECALC_RANGE; i < uParams->audioBufferSize; i++) {
      smoothBuf[i] = (logSmooth * smoothBuf[i-1] + (1.0f - logSmooth) * smoothBuf[i]);
      if (i < begin) continue;
      if (useMin) {
        if (smoothBuf[i] < smoothPeak) {
          smoothPeak = smoothBuf[i];
        }
      } else {
        if (smoothBuf[i] > smoothPeak) {
          smoothPeak = smoothBuf[i];
        }
      }
    }
  }
  // locate trigger
  bool triggerHigh = false, triggerLow = false, foundTrigger = false;
  triggerLevel = smoothPeak * params[1].getValue<float>();
  triggerIndex = begin;

  while (triggerIndex > 0) {
    triggerIndex--;
    if (smoothBuf[triggerIndex + windowSize/2] < triggerLevel) {
      triggerLow = true;
      CHECK_TRIGGERED;
      if (foundTrigger) break;
    }
    if (smoothBuf[triggerIndex + windowSize/2] > triggerLevel) {
      triggerHigh = true;
      CHECK_TRIGGERED;
    }
    if (triggerIndex < uParams->audioBufferSize - 2 * windowSize) return false; // out of window
  }

  if (triggerIndex > begin) {
    alignRegionSize = uParams->audioBufferSize - triggerIndex;
  } else {
    alignRegionSize = windowSize;
  }

  triggered = true;
  return true;
}

nint TriggerSmooth::getAlignRegionSize() {
  return alignRegionSize;
}

float* TriggerSmooth::getSmoothBuffer() {
  return smoothBuf;
}

float TriggerSmooth::getTriggerLevel() {
  return triggerLevel;
}

nint TriggerSmooth::getTriggerIndex() {
  return triggerIndex;
}

bool TriggerSmooth::getTriggered() {
  return triggered;
}

TriggerSmooth::~TriggerSmooth() {
  for (TriggerParam i:params) i.destroy();
  DELETE_PTR_ARR(smoothBuf)
}
