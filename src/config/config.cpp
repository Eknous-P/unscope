#include "config.h"
#include "imgui_toggle.h"
#include "imgui_stdlib.h"

#define CHAR_BUF_SIZE 2048

Setting::Setting(SettingTypes type, const char* k, const char* l, const char* d) {
  switch (type) {
    case SETTING_TOGGLE:
      data = new bool;
      break;
    case SETTING_INT:
    case SETTING_NONE:
      data = new int;
      break;
    case SETTING_FLOAT:
      data = new float;
      break;
    case SETTING_STRING:
      data = new char[CHAR_BUF_SIZE];
      break;
    default: return;
  }

  key = k;
  label = l;
  desc = d;
}

bool Setting::passesFilter(ImGuiTextFilter* filter) {
  // TODO: key and desc too
  return filter->PassFilter(label);
}

void Setting::draw() {
  switch (type) {
    case SETTING_TOGGLE:
      ImGui::Toggle(label, (bool*)data);
      break;
    case SETTING_INT:
    case SETTING_NONE:
      ImGui::InputScalar(label, ImGuiDataType_S32, data, &step_one, NULL, "%d");
      break;
    case SETTING_FLOAT:
      ImGui::InputScalar(label, ImGuiDataType_Float, data, &step_one, NULL, "%g");
      break;
    case SETTING_STRING:
      ImGui::InputText(label, (char*)data, CHAR_BUF_SIZE);
      break;
    default: return;
  }
  if (desc) {
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("%s", desc);
    }
  }
}

int Setting::save(YAML::Node* conf) {
  switch (type) {
    case SETTING_TOGGLE:
      (*conf)[key] = *(bool*)data;
      return 0;
    case SETTING_INT:
    case SETTING_NONE:
      (*conf)[key] = *(int*)data;
      return 0;
    case SETTING_FLOAT:
      (*conf)[key] = *(float*)data;
      return 0;
    case SETTING_STRING:
      (*conf)[key] = (char*)data;
      return 0;
    default: return 0;
  }
}

int Setting::load(YAML::Node* conf) {
  switch (type) {
    case SETTING_TOGGLE:
      *(bool*)data = (*conf)[key].as<bool>();
      return 0;
    case SETTING_INT:
    case SETTING_NONE:
      *(bool*)data = (*conf)[key].as<int>();
      return 0;
    case SETTING_FLOAT:
      *(bool*)data = (*conf)[key].as<float>();
      return 0;
    case SETTING_STRING:
      snprintf((char*)data, CHAR_BUF_SIZE, "%s", ((*conf)[key].as<std::string>()).c_str());
      return 0;
    default: return 0;
  }
}

Setting::~Setting() {
  switch (type) {
    case SETTING_TOGGLE:
      delete (bool*)data;
      break;
    case SETTING_INT:
    case SETTING_NONE:
      delete (int*)data;
      break;
    case SETTING_FLOAT:
      delete (float*)data;
      break;
    case SETTING_STRING:
      delete[] (char*)data;
      break;
    default: return;
  }
}