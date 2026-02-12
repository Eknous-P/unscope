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

#include "shared.h"
#include "param.h"
#include "imgui.h"
#include "imgui-knobs.h"
#include "imgui_toggle.h"
#include <vector>

constexpr int defaultInputIntLimits[2]={0,16};
constexpr float defaultInputFloatLimits[2]={0.0f,1.0f};

const char* Parameter::getLabel() {
  return label;
}

void *Parameter::getValuePtr() {
  return valuePtr;
}

bool Parameter::draw() {
  hovered=false;
  active=false;
  bool ret=false;
  ImGui::PushID(label);
  switch (type) {
    case PARAM_TOGGLE: {
      ret=ImGui::Toggle(label, (bool*)valuePtr);
      break;
    }
    case PARAM_KNOBNORM: {
      ret=ImGuiKnobs::Knob(label, (float*)valuePtr, -1.0f, 1.0f, 0.0f, "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
      if (exactInput) RIGHTCLICK_EXACT_INPUT((float*)valuePtr, ImGuiDataType_Float, {
        if (getValue<float>() > 1.0f) setValue<float>( 1.0f);
        if (getValue<float>() <-1.0f) setValue<float>(-1.0f);
      });
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) setValue<float>(0.0f);
      break;
    }
    case PARAM_KNOBUNIT: {
      ret=ImGuiKnobs::Knob(label, (float*)valuePtr, 0.0f, 1.0f, 0.0f, "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
      if (exactInput) RIGHTCLICK_EXACT_INPUT((float*)valuePtr, ImGuiDataType_Float, {
        if (getValue<float>() > 1.0f) setValue<float>(1.0f);
        if (getValue<float>() < 0.0f) setValue<float>(0.0f);
      });
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) setValue<float>(0.0f);
      break;
    }

    case PARAM_INPUTINT: {
      int* limits=(int*)paramData;
      if (limits==NULL) limits=(int*)defaultInputIntLimits;
      ret=ImGui::InputInt(label, (int*)valuePtr);
      if (ret) {
        if (getValue<int>() > limits[1]) setValue<int>(limits[1]);
        if (getValue<int>() < limits[0]) setValue<int>(limits[0]);
      }
      break;
    }
    case PARAM_INPUTFLOAT: {
      float* limits=(float*)paramData;
      if (limits==NULL) limits=(float*)defaultInputIntLimits;
      ret=ImGui::InputFloat(label, (float*)valuePtr);
      if (ret) {
        if (getValue<float>() > limits[1]) setValue<float>(limits[1]);
        if (getValue<float>() < limits[0]) setValue<float>(limits[0]);
      }
      break;
    }
    case PARAM_COMBO_CSTR: {
      const char** entries=(const char**)paramData;
      if (entries==NULL) {
        ImGui::Text("no combo data!");
        break;
      }
      if (ImGui::BeginCombo(label, entries[getValue<int>()])) {
        for (int i=0; entries[i]; i++) {
          if (ImGui::Selectable(entries[i], i==getValue<int>())) {
            setValue<int>(i);
          }
        }
        ImGui::EndCombo();
      }
      break;
    }
    case PARAM_COMBOV_STR: {
      if (paramData==NULL) {
        ImGui::Text("no combo data!");
        break;
      }
      vector<string> entries = *(vector<string>*)paramData;
      if (entries.size()==0) {
        ImGui::TextUnformatted(label);
        break;
      }
      if (ImGui::BeginCombo(label, entries[getValue<int>()].c_str())) {
        for (int i=0; i<entries.size(); i++) {
          if (ImGui::Selectable(entries[i].c_str(), i==getValue<int>())) {
            setValue<int>(i);
          }
        }
        ImGui::EndCombo();
      }
      break;
    }
    case PARAM_COMBO_INT: {
      if (paramData==NULL) {
        ImGui::Text("no combo data!");
        break;
      }
      int len = *(int*)paramData;
      int* entries = &(((int*)paramData)[1]);
      char strbuf[64];
      snprintf(strbuf, 64, "%d", entries[getValue<int>()]);
      if (ImGui::BeginCombo(label, strbuf)) {
        for (int i=0; i<len; i++) {
          snprintf(strbuf, 64, "%d", entries[i]);
          if (ImGui::Selectable(strbuf, i==getValue<int>())) {
            setValue<int>(i);
          }
        }
        ImGui::EndCombo();
      }
      break;
    }
    default:
      ImGui::Text("what?");
      break;
  }
  if (ImGui::IsItemHovered()) hovered=true;
  if (ImGui::IsItemActive()) active=true;
  if (desc) {
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
      ImGui::SetTooltip("%s", desc);
    }
  }
  ImGui::PopID();
  return ret;
}

ParamTypes Parameter::getType() {
  return type;
}

bool Parameter::isHovered() {
  return hovered;
}

bool Parameter::isActive() {
  return active;
}

Parameter::Parameter():
  type(PARAM_NONE),
  valuePtr(NULL),
  label(NULL),
  desc(NULL),
  exactInput(false),
  hovered(false),
  active(false) {}

Parameter::Parameter(ParamTypes t, bool i, const char* l, void* ext, void* value):
  desc(NULL),
  hovered(false),
  active(false) {
  type       = t;
  exactInput = i;
  label      = l;
  paramData  = ext;
  if (value) {
    valuePtr = value;
    ownValue = false;
  } else {
    INIT_PARAM_VALUE
    ownValue = true;
  }
}

Parameter::Parameter(ParamTypes t, bool i, const char* l, const char* d, void* ext, void* value):
  hovered(false),
  active(false) {
  type       = t;
  exactInput = i;
  label      = l;
  desc       = d;
  paramData  = ext;
  if (value) {
    valuePtr = value;
    ownValue = false;
  } else {
    INIT_PARAM_VALUE
    ownValue = true;
  }
}

void Parameter::destroy() {
  if (!ownValue) return;
  switch (type) {
    case PARAM_TOGGLE:
      if (valuePtr) delete (bool*)valuePtr;
      break;
    case PARAM_KNOBNORM:
    case PARAM_KNOBUNIT:
    case PARAM_INPUTFLOAT:
      if (valuePtr) delete (float*)valuePtr;
      break;
    case PARAM_INPUTINT:
    case PARAM_COMBO_CSTR:
    case PARAM_COMBOV_STR:
    case PARAM_COMBO_INT:
    default:
      if (valuePtr) delete (int*)valuePtr;
      break;
  }
}

