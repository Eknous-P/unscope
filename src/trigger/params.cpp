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

#include "trigger.h"
#include "imgui.h"
#include "imgui-knobs.h"
#include "imgui_toggle.h"

#define INIT_PARAM_VALUE \
  switch (type) { \
    case TP_TOGGLE: \
      valuePtr = new bool; \
      break; \
    case TP_KNOBNORM: \
    case TP_KNOBUNIT: \
      valuePtr = new float; \
      break; \
    default: \
      valuePtr = new int; \
      break; \
  } \
  memset(valuePtr, 0, TriggerParamTypeSize[type]);

const char* TriggerParam::getLabel() {
  return label;
}

void *TriggerParam::getValuePtr() {
  return valuePtr;
}

bool TriggerParam::draw() {
  hovered=false;
  active=false;
  bool ret=false;
  ImGui::PushID(label);
  switch (type) {
    case TP_TOGGLE: {
      ret=ImGui::Toggle(label, (bool*)valuePtr);
      break;
    }
    case TP_KNOBNORM: {
      ret=ImGuiKnobs::Knob(label, (float*)valuePtr, -1.0f, 1.0f, 0.0f, "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
      if (exactInput) RIGHTCLICK_EXACT_INPUT((float*)valuePtr, ImGuiDataType_Float, {
        if (getValue<float>() > 1.0f) setValue<float>( 1.0f);
        if (getValue<float>() <-1.0f) setValue<float>(-1.0f);
      });
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) setValue<float>(0.0f);
      break;
    }
    case TP_KNOBUNIT: {
      ret=ImGuiKnobs::Knob(label, (float*)valuePtr, 0.0f, 1.0f, 0.0f, "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
      if (exactInput) RIGHTCLICK_EXACT_INPUT((float*)valuePtr, ImGuiDataType_Float, {
        if (getValue<float>() > 1.0f) setValue<float>(1.0f);
        if (getValue<float>() < 0.0f) setValue<float>(0.0f);
      });
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) setValue<float>(0.0f);
      break;
    }
    case TP_NONE:
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

TriggerParamTypes TriggerParam::getType() {
  return type;
}

bool TriggerParam::isHovered() {
  return hovered;
}

bool TriggerParam::isActive() {
  return active;
}

TriggerParam::TriggerParam():
  type(TP_NONE),
  valuePtr(NULL),
  label(NULL),
  desc(NULL),
  exactInput(false),
  hovered(false),
  active(false) {}

TriggerParam::TriggerParam(TriggerParamTypes t, bool i, const char* l, bool dragX, bool dragY):
  desc(NULL),
  hovered(false),
  active(false) {
  type       = t;
  exactInput = i;
  label      = l;
  bindToDragX = dragX;
  bindToDragY = dragY;
  INIT_PARAM_VALUE
}

TriggerParam::TriggerParam(TriggerParamTypes t, bool i, const char* l, const char* d, bool dragX, bool dragY):
  hovered(false),
  active(false) {
  type       = t;
  exactInput = i;
  label      = l;
  desc       = d;
  bindToDragX = dragX;
  bindToDragY = dragY;
  INIT_PARAM_VALUE
}

void TriggerParam::destroy() {
  switch (type) {
    case TP_TOGGLE:
      if (valuePtr) delete (bool*)valuePtr;
      break;
    case TP_KNOBNORM:
    case TP_KNOBUNIT:
      if (valuePtr) delete (float*)valuePtr;
      break;
    default:
      if (valuePtr) delete (int*)valuePtr;
      break;
  }
}
