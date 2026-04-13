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

#ifndef PARAM_H
#define PARAM_H

#include <cstddef>
#include "config.h"

enum ParamTypes : unsigned char {
  PARAM_NONE = 0,
  PARAM_TOGGLE,
  PARAM_KNOBFLOAT,  // EXT: [min, max, default]
  PARAM_KNOBNORM,   // [-1,1]
  PARAM_KNOBUNIT,   // [ 0,1]

  PARAM_INPUTINT,   // EXT: [min, max]
  PARAM_INPUTFLOAT, // EXT: [min, max]

  PARAM_COMBO_CSTR, // EXT: ["c1", "c2" ... NULL]
  PARAM_COMBO_INT,  // EXT: [len, ...]
  PARAM_COMBOV_STR, // EXT: pointer to std::vector<string>

  PARAM_TEXTTOGGLE, // EXT: ["c1", "c2"]

  PARAM_COLOR,

  PARAM_MAX
};

class Parameter {
  protected:
    void *valuePtr, *paramData, *defaultValue;
    const char *internalName, *label, *desc;
    ParamTypes type;
    bool exactInput;
    bool hovered, active, ownValue;
  public:
    const char* getLabel();
    void *getValuePtr();

    template<typename T>
    T getValue() const {return *(T*)valuePtr;}

    template<typename T>
    void setValue(T v) {*(T*)valuePtr = v;}

    bool draw();
    ParamTypes getType();
    bool isHovered();
    bool isActive();

    void readFromConfig(USCConfig* conf);
    void readFromConfig(YAML::Node& node);
    void writeToConfig(USCConfig* conf);
    void writeToConfig(YAML::Node& node);

    float getEstimatedWidth();

    Parameter();
    Parameter(ParamTypes t, bool i, const char* n, const char* l, const char* d=NULL, void* ext=NULL, void* value=NULL, void* defV=NULL);
    void destroy();
};

#define INIT_PARAM_VALUE \
  switch (type) { \
    case PARAM_TOGGLE: \
    case PARAM_TEXTTOGGLE: \
      valuePtr = new bool; \
      setValue<bool>(false); \
      break; \
    case PARAM_KNOBFLOAT: \
    case PARAM_KNOBNORM: \
    case PARAM_KNOBUNIT: \
    case PARAM_INPUTFLOAT: \
      valuePtr = new float; \
      setValue<float>(0.0f); \
      break; \
    case PARAM_COLOR: \
      valuePtr = new unsigned int; \
      setValue<unsigned int>(0); \
      break; \
    case PARAM_INPUTINT: \
    case PARAM_COMBO_CSTR: \
    case PARAM_COMBOV_STR: \
    case PARAM_COMBO_INT: \
    default: \
      valuePtr = new int; \
      setValue<int>(0); \
      break; \
  } \

extern const int defaultInputIntLimits[2];
extern const float defaultInputFloatLimits[2];

#ifdef PROGRAM_DEBUG
extern const char* paramTypeNames[];
#endif

#endif
