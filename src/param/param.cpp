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
#include "imgui_internal.h"
#include "imgui-knobs.h"
#include "imgui_toggle.h"

namespace ImGui {
  bool BetterKnob(const char* label, float* v, float vMin, float vMax, float speed, const char* fmt, ImGuiKnobVariant variant, float knobSize, ImGuiKnobFlags flags, int steps) {
    const float textSize = ImGui::CalcTextSize(label).x;
    const float alignment = fabs(textSize-knobSize)/2.0f;
    bool ret=false;
    ImGui::BeginGroup();
    if (textSize > knobSize) {
      ImGui::TextUnformatted(label);
      ImGui::SetCursorPosX(ImGui::GetCursorPosX()+alignment);
      ret = ImGuiKnobs::Knob(label, v, vMin, vMax, speed, fmt, variant, knobSize, flags|ImGuiKnobFlags_NoTitle, steps);
    } else {
      ImGui::SetCursorPosX(ImGui::GetCursorPosX()+alignment);
      ImGui::TextUnformatted(label);
      ret = ImGuiKnobs::Knob(label, v, vMin, vMax, speed, fmt, variant, knobSize, flags|ImGuiKnobFlags_NoTitle, steps);
    }
    ImGui::EndGroup();
    return ret;
  }
}


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
    case PARAM_TEXTTOGGLE: {
      if (paramData == NULL) {
        ImGui::Text("no param data!");
        break;
      }
      const char** values = (const char**)paramData;
      ImGui::AlignTextToFramePadding();
      ret=ImGui::Button(values[getValue<bool>()]);
      if (ret) {
        setValue<bool>(!getValue<bool>());
      }
      if (label) {
        ImGui::SameLine();
        ImGui::TextUnformatted(label);
      }
      break;
    }
    case PARAM_KNOBNORM:
    case PARAM_KNOBUNIT:
    case PARAM_KNOBFLOAT: {
      float knobMin, knobMax, knobDefault;
      switch (type) {
        case PARAM_KNOBFLOAT: {
          if (paramData == NULL) assert("no knob limits set!");
          float* knobRanges = (float*)paramData;
          knobMin = knobRanges[0];
          knobMax = knobRanges[1];
          knobDefault = knobRanges[2];
          break;
        }
        case PARAM_KNOBNORM:
          knobMin =-1.0f; knobMax = 1.0f; knobDefault = 0.0f;
          break;
        case PARAM_KNOBUNIT:
        default:
          knobMin = 0.0f; knobMax = 1.0f; knobDefault = 0.0f;
          break;
      }
      ret=ImGui::BetterKnob(label, (float*)valuePtr, knobMin, knobMax, 0.0f, "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
      if (exactInput) RIGHTCLICK_EXACT_INPUT((float*)valuePtr, ImGuiDataType_Float, {
        if (getValue<float>() > knobMax) setValue<float>(knobMax);
        if (getValue<float>() < knobMin) setValue<float>(knobMin);
        ret=true;
      });
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        setValue<float>(knobDefault);
        ret=true;
      }
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
            ret=true;
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
            ret=true;
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
            ret=true;
          }
        }
        ImGui::EndCombo();
      }
      break;
    }
    case PARAM_COLOR: {
      char strbuf[256];
      ImGui::TextUnformatted(label);
      ImGui::SameLine();
      snprintf(strbuf, 256, "%s_colorButton", internalName);
      ImVec4 col = ImGui::ColorConvertU32ToFloat4(*(unsigned int*)valuePtr);
      ret=ImGui::ColorButton(strbuf, col);
      snprintf(strbuf, 256, "##%s_color", internalName);
      if (ImGui::BeginPopupContextItem(strbuf,ImGuiPopupFlags_MouseButtonLeft)) {
        snprintf(strbuf, 256, "##%s_colorEditor", internalName);
        if (ImGui::ColorPicker4(strbuf, (float*)&col)) {
          setValue<unsigned int>(ImGui::ColorConvertFloat4ToU32(col));
        }
        ImGui::EndPopup();
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

void Parameter::readFromConfig(USCConfig* conf) {
  switch (type) {
    case PARAM_TOGGLE:
    case PARAM_TEXTTOGGLE:
      setValue<bool>(
        conf->getConfig<bool>(internalName,
        defaultValue?*(bool*)defaultValue:false
      ));
      return;
    case PARAM_KNOBFLOAT:
    case PARAM_KNOBNORM:
    case PARAM_KNOBUNIT:
    case PARAM_INPUTFLOAT:
      setValue<float>(
        conf->getConfig<float>(internalName,
        defaultValue?*(float*)defaultValue:0.0f
      ));
      return;
    case PARAM_COLOR:
      setValue<unsigned int>(
        conf->getConfig<unsigned int>(internalName,
        defaultValue?*(unsigned int*)defaultValue:0
      ));
      return;
    case PARAM_COMBOV_STR: {
      string value = defaultValue?*(string*)defaultValue:"";
      if (conf->exists(internalName))
        value = conf->getConfig<string>(internalName, "");
      vector<string>* entries = (vector<string>*)paramData;
      for (int i=0; i<entries->size(); i++) {
        if (entries->at(i) == value) {
          setValue(i);
          return;
        }
      }
      setValue<int>(0);
      return;
    }
    case PARAM_INPUTINT:
    case PARAM_COMBO_CSTR:
    case PARAM_COMBO_INT:
      setValue<int>(
        conf->getConfig<int>(internalName,
        defaultValue?*(int*)defaultValue:0
      ));
      return;
    default: return;
  }
}

void Parameter::readFromConfig(YAML::Node& node) {
  switch (type) {
    case PARAM_TOGGLE:
    case PARAM_TEXTTOGGLE:
      if (node[internalName])
        setValue<bool>(node[internalName].as<bool>());
      else
        setValue<bool>(defaultValue?*(bool*)defaultValue:false);
      return;
    case PARAM_KNOBFLOAT:
    case PARAM_KNOBNORM:
    case PARAM_KNOBUNIT:
    case PARAM_INPUTFLOAT:
      if (node[internalName])
        setValue<float>(node[internalName].as<float>());
      else
        setValue<float>(defaultValue?*(float*)defaultValue:0.0f);
      return;
    case PARAM_COLOR:
      if (node[internalName])
        setValue<unsigned int>(node[internalName].as<unsigned int>());
      else
        setValue<unsigned int>(defaultValue?*(unsigned int*)defaultValue:0);
      return;
    case PARAM_COMBOV_STR: {
      string value = defaultValue?*(string*)defaultValue:"";
      if (node[internalName])
        value = node[internalName].as<string>();
      vector<string>* entries = (vector<string>*)paramData;
      for (int i=0; i<entries->size(); i++) {
        if (entries->at(i) == value) {
          setValue(i);
          return;
        }
      }
      setValue<int>(0);
      return;
    }
    case PARAM_COMBO_CSTR:
    case PARAM_COMBO_INT:
    case PARAM_INPUTINT:
    default:
      if (node[internalName])
        setValue<int>(node[internalName].as<int>());
      else
        setValue<int>(defaultValue?*(int*)defaultValue:0);
      return;
  }
}

void Parameter::writeToConfig(USCConfig* conf) {
  switch (type) {
    case PARAM_TOGGLE:
    case PARAM_TEXTTOGGLE:
      conf->setConfig<bool>(internalName, getValue<bool>());
      return;
    case PARAM_KNOBFLOAT:
    case PARAM_KNOBNORM:
    case PARAM_KNOBUNIT:
    case PARAM_INPUTFLOAT:
      conf->setConfig<float>(internalName, getValue<float>());
      return;
    case PARAM_COLOR:
      conf->setConfig<unsigned int>(internalName, getValue<unsigned int>());
      return;
      conf->setConfig<const char*>(internalName, ((const char**)paramData)[getValue<int>()]);
      return;
    case PARAM_COMBOV_STR:
      conf->setConfig<string>(internalName, ((vector<string>*)paramData)->at(getValue<int>()));
      return;
    case PARAM_COMBO_INT:  // a int array isnt going to change
    case PARAM_COMBO_CSTR: // a const char* array isnt going to change
    case PARAM_INPUTINT:
    default:
      conf->setConfig<int>(internalName, getValue<int>());
      return;
  }
}

void Parameter::writeToConfig(YAML::Node& node) {
  switch (type) {
    case PARAM_TOGGLE:
    case PARAM_TEXTTOGGLE:
      node[internalName] = getValue<bool>();
      return;
    case PARAM_KNOBFLOAT:
    case PARAM_KNOBNORM:
    case PARAM_KNOBUNIT:
    case PARAM_INPUTFLOAT:
      node[internalName] = getValue<float>();
      return;
    case PARAM_COLOR:
      node[internalName] = getValue<unsigned int>();
      return;
    case PARAM_COMBOV_STR:
      node[internalName] = ((vector<string>*)paramData)->at(getValue<int>());
      return;
    case PARAM_INPUTINT:
    case PARAM_COMBO_CSTR:
    case PARAM_COMBO_INT:
      node[internalName] = getValue<int>();
      return;
    default: return;
  }
}

float Parameter::getEstimatedWidth() {
  switch (type) {
    case PARAM_TOGGLE:
      return
        ImGui::GetFrameHeight()*1.618f + // toggle width
        ImGui::GetStyle().ItemInnerSpacing.x + // inner spacing
        ImGui::CalcTextSize(label).x; // label size
    case PARAM_TEXTTOGGLE:
      return
        ImGui::GetStyle().FramePadding.x*2.0f + // padding
        ImGui::CalcTextSize(label).x;
    case PARAM_KNOBNORM:
    case PARAM_KNOBUNIT:
    case PARAM_KNOBFLOAT:
      return
        ImGui::GetStyle().FramePadding.x*2.0f +
        ImMax(KNOBS_SIZE, ImGui::CalcTextSize(label).x);
    default:
      return 0;
  }
}

Parameter::Parameter():
  type(PARAM_NONE),
  valuePtr(NULL),
  paramData(NULL),
  defaultValue(NULL),
  internalName(NULL),
  label(NULL),
  desc(NULL),
  exactInput(false),
  hovered(false),
  active(false),
  ownValue(false) {}

Parameter::Parameter(ParamTypes t, bool i, const char* n, const char* l, const char* d, void* ext, void* value, void* defV):
  hovered(false),
  active(false) {
  type       = t;
  exactInput = i;
  internalName = n;
  label      = l;
  desc       = d;
  paramData  = ext;
  defaultValue = defV;
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
    case PARAM_TEXTTOGGLE:
      if (valuePtr) {
        delete (bool*)valuePtr;
        valuePtr=NULL;
      }
      break;
    case PARAM_KNOBFLOAT:
    case PARAM_KNOBNORM:
    case PARAM_KNOBUNIT:
    case PARAM_INPUTFLOAT:
      if (valuePtr) {
        delete (float*)valuePtr;
        valuePtr=NULL;
      }
      break;
    case PARAM_COLOR:
      if (valuePtr) {
        delete (unsigned int*) valuePtr;
        valuePtr=NULL;
      }
    case PARAM_INPUTINT:
    case PARAM_COMBO_CSTR:
    case PARAM_COMBOV_STR:
    case PARAM_COMBO_INT:
    default:
      if (valuePtr) {
        delete (int*)valuePtr;
        valuePtr=NULL;
      }
      break;
  }
}

#ifdef PROGRAM_DEBUG
const char* paramTypeNames[] = {
  "none",
  "toggle",
  "knob",
  "knob normalized",
  "knob unit",
  "int input",
  "float input",
  "c string combo",
  "int combo",
  "string combo",
  "text toggle",
  "color picker"
};
#endif

