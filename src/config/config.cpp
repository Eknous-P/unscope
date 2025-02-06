#include "config.h"
#include "imgui_toggle.h"
#include "imgui_stdlib.h"

#define CHAR_BUF_SIZE 2048

Setting::Setting(SettingTypes t, const char* k, const char* l, const char* d):
  isBound(false) {
  type = t;
  switch (type) {
    case SETTING_TOGGLE:
      data = new bool;
      if (!data) return;
      *(bool*)data = false;
      break;
    case SETTING_INT:
    case SETTING_NONE:
      data = new int;
      if (!data) return;
      *(int*)data = 0;
      break;
    case SETTING_FLOAT:
      data = new float;
      if (!data) return;
      *(int*)data = 0;
      break;
    case SETTING_STRING:
      data = new char[CHAR_BUF_SIZE];
      if (!data) return;
      memset(data, 0, CHAR_BUF_SIZE * sizeof(char));
      break;
    default: return;
  }

  key = k;
  label = l;
  desc = d;
}

Setting::Setting(SettingTypes t, const char* k, const char* l, const char* d, void* bind):
  isBound(true) {
  type = t;
  data = bind;
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

void* Setting::getData() {
  return data;
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

void Setting::destroy() {
  if (isBound) return;
  switch (type) {
    case SETTING_TOGGLE:
      if (data) delete (bool*)data;
      break;
    case SETTING_INT:
    case SETTING_NONE:
      if (data) delete (int*)data;
      break;
    case SETTING_FLOAT:
      if (data) delete (float*)data;
      break;
    case SETTING_STRING:
      if (data) delete[] (char*)data;
      break;
    default: return;
  }
  data = NULL;
}

// USCConfig definitions

USCConfig::USCConfig(const char* filePath, unscopeParams* p) {
  try {
    conf = YAML::LoadFile(filePath);
    isEmpty = false;
  } catch (std::exception& xc) {
    printf(ERROR_MSG "config file not found (%s)" MSG_END, xc.what());
    // TODO: DEAL WITH CONFIG CREATION
    conf = YAML::Load("");
    isEmpty = true;
  }

  settings = {
    Setting(SETTING_INT, "channels", "channels", NULL, &(p->channels)),
    Setting(SETTING_INT, "sampleRate", "sample rate", NULL, &(p->sampleRate)),
    Setting(SETTING_INT, "bufferSize", "buffer size", NULL, &(p->audioBufferSize)),
    Setting(SETTING_INT, "frameSize", "frame size", NULL, &(p->audioFrameSize)),

  };
}

int USCConfig::loadConfig() {
  if (isEmpty) return 1;
  for (Setting s:settings) s.load(&conf);
  return 0;
}

void USCConfig::drawSettings() {
  for (Setting s:settings) s.draw();
}

USCConfig::~USCConfig() {
  for (Setting s:settings) s.destroy();
  (void)conf;
}