#include "config.h"
#include "extData.h"
#include "imgui_toggle.h"
#include <cstdio>
#include <emitter.h>
#include <emittermanip.h>
#include <imgui.h>
#include <node/parse.h>
#include <sys/stat.h>
#include <errno.h>

#define CHAR_BUF_SIZE 4096

Setting::Setting(SettingTypes t, const char* k, const char* l, const char* d, void* bind, void* ext, size_t es) {
  if (bind) {
    data = bind;
    isBound = true;
  } else {
    switch (t) {
      case SETTING_TOGGLE:
        data = new bool;
        if (!data) return;
        *(bool*)data = false;
        break;
      case SETTING_NONE:
      case SETTING_INT:
      case SETTING_SELECTABLE_INT:
      case SETTING_SELECTABLE_STRING:
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
      case SETTING_COLOR:
        data = new ImVec4;
        if (!data) return;
        memset(data, 0, sizeof(ImVec4));
      default: break;
    }
    isBound = false;
  }
  type = t;
  key = k;
  label = l;
  desc = d;
  extData = ext;
  extDataSize = es;
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
    case SETTING_SELECTABLE_INT: {
      char buf[64];
      snprintf(buf, 64, "%d", *(int*)data);
      if (ImGui::BeginCombo(label, buf)) {
        for (size_t i=0; i<extDataSize; i++) {
          snprintf(buf, 64, "%d", ((int*)extData)[i]);
          if (ImGui::Selectable(buf, *(int*)data == ((int*)extData)[i])) {
            *(int*)data = ((int*)extData)[i];
          }
        }
        ImGui::EndCombo();
      }
      break;
    }
    case SETTING_SELECTABLE_STRING: {
      if (ImGui::BeginCombo(label, ((char**)extData)[*(int*)data])) {
        for (size_t i=0; i<extDataSize; i++) {
          if (ImGui::Selectable(((char**)extData)[i], *(int*)data == i)) {
            *(int*)data = i;
          }
        }
        ImGui::EndCombo();
      }
      break;
    }
    case SETTING_COLOR: {
      ImVec4 col = ImGui::ColorConvertU32ToFloat4(*(ImU32*)data);
      if (ImGui::ColorEdit4(label, (float*)&col)) {
        *(ImU32*)data = ImGui::ColorConvertFloat4ToU32(col);
      }
      break;
    }
    default: return;
  }
  if (desc) {
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
      ImGui::SetTooltip("%s", desc);
    }
  }
}

void* Setting::getData() {
  return data;
}

int Setting::save(YAML::Node* conf) {
  // if (conf->Type() != YAML::NodeType::Map) return 1;
  switch (type) {
    case SETTING_TOGGLE: {
      if ((*conf)[key]) {
        (*conf)[key] = *(bool*)data;
      } else {
        (*conf).force_insert(key, *(bool*)data);
      }
      return 0;
    }
    case SETTING_NONE:
    case SETTING_INT:
    case SETTING_SELECTABLE_INT:
    case SETTING_SELECTABLE_STRING:
    case SETTING_COLOR: {
      if ((*conf)[key]) {
        (*conf)[key] = *(int*)data;
      } else {
        (*conf).force_insert(key, *(int*)data);
      }
      return 0;
    }
    case SETTING_FLOAT: {
      if ((*conf)[key]) {
        (*conf)[key] = *(float*)data;
      } else {
        (*conf).force_insert(key, *(float*)data);
      }
      return 0;
    }
    case SETTING_STRING: {
      if ((*conf)[key]) {
        (*conf)[key] = (char*)data;
      } else {
        (*conf).force_insert(key, (char*)data);
      }
      return 0;
    }
    default: return 0;
  }
}

int Setting::load(YAML::Node* conf) {
  YAML::Node dat = (*conf)[key];
  switch (type) {
    case SETTING_TOGGLE:
      *(bool*)data = dat.as<bool>();
      return 0;
    case SETTING_INT:
    case SETTING_NONE:
    case SETTING_SELECTABLE_INT:
    case SETTING_SELECTABLE_STRING:
    case SETTING_COLOR:
      *(bool*)data = dat.as<signed int>();
      return 0;
    case SETTING_FLOAT:
      *(bool*)data = dat.as<float>();
      return 0;
    case SETTING_STRING:
      snprintf((char*)data, CHAR_BUF_SIZE, "%s", (dat.as<std::string>()).c_str());
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
    case SETTING_NONE:
    case SETTING_INT:
    case SETTING_SELECTABLE_INT:
    case SETTING_SELECTABLE_STRING:
    case SETTING_COLOR:
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
  confFile = filePath;
  params = p;
  YAML::Node temp;
  try {
    temp = YAML::LoadFile(filePath);
    conf = temp;
  } catch (std::exception& xc) {
    printf(ERROR_MSG "config file not found (%s)" MSG_END, xc.what());
    printf(INFO_MSG "new config file will be made at %s" MSG_END, confFile);
  }

  settings = {
    SettingsCategory("Audio", {
      Setting(SETTING_INT, "channels", "channels", NULL, &(params->channels), NULL, 0),
      Setting(SETTING_SELECTABLE_INT, "sampleRate", "sample rate", NULL, &(params->sampleRate), (void*)sampleRateSelectableData, 9),
      Setting(SETTING_INT, "bufferSize", "buffer size", NULL, &(params->audioBufferSize), NULL, 0),
      Setting(SETTING_INT, "frameSize", "frame size", NULL, &(params->audioFrameSize), NULL, 0),
    }),
    SettingsCategory("Colors", {
      Setting(SETTING_COLOR, "chan1col", "channel 1", NULL, &(params->chanColor[0]), NULL, 0),
    })
  };
}

int USCConfig::loadConfig() {
  if (isEmpty) return 1;
  for (SettingsCategory c:settings) {
    for (Setting s:c.settings) s.load(&conf);
  }
  return 0;
}

int USCConfig::saveConfig() {
  for (SettingsCategory c:settings) {
    for (Setting s:c.settings) s.save(&conf);
  }
  YAML::Emitter confOut;
  confOut << conf;
  FILE* f;
  // attempt 1: check for file
  f = fopen(confFile, "wb");
  if (!f) {
    printf(ERROR_MSG "failed to open config file! %d" MSG_END, errno);
    printf(INFO_MSG "making conf dir... (%d)" MSG_END,
    // attempt 2: make the file
    mkdir(confFile, 0733));
    f = fopen(confFile, "wb");
    if (!f) {
      printf(ERROR_MSG "really failed to open config file!!! %d" MSG_END, errno);
      return 1;
    }
  }
  rewind(f);
  fprintf(f,"%s",confOut.c_str());
  fclose(f);
  printf(SUCCESS_MSG "written config!!" MSG_END);
  return 0;
}

void USCConfig::drawSettings() {
  for (SettingsCategory c:settings) {
    ImGui::BulletText("%s",c.name);
    ImGui::Indent();
    for (Setting s:c.settings) s.draw();
    ImGui::Unindent();
  }
}

USCConfig::~USCConfig() {
  for (SettingsCategory c:settings) {
    for (Setting s:c.settings) s.destroy();
  }
  (void)conf;
}