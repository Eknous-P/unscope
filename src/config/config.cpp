#include "config.h"
#include "extData.h"
#include "imgui_toggle.h"
#include <sys/stat.h>

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
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
      ImGui::SetTooltip("%s", desc);
    }
  }
}

void* Setting::getData() {
  return data;
}

int Setting::save(YAML::Node* conf) {
  switch (type) {
    case SETTING_TOGGLE: {
      bool value = *(bool*)data;
      if ((*conf)[key]) {
        (*conf)[key] = value;
      } else {
        (*conf).force_insert(key, value);
      }
      return 0;
    }
    case SETTING_NONE:
    case SETTING_INT:
    case SETTING_SELECTABLE_INT:
    case SETTING_SELECTABLE_STRING:
    case SETTING_COLOR: {
      int value = *(int*)data;
      if ((*conf)[key]) {
        (*conf)[key] = value;
      } else {
        (*conf).force_insert(key, value);
      }
      return 0;
    }
    case SETTING_FLOAT: {
      float value = *(float*)data;
      if ((*conf)[key]) {
        (*conf)[key] = value;
      } else {
        (*conf).force_insert(key, value);
      }
      return 0;
    }
    case SETTING_STRING: {
      const char* value = (const char*)data;
      if ((*conf)[key]) {
        (*conf)[key] = value;
      } else {
        (*conf).force_insert(key, value);
      }
    }
    default: return 0;
  }
}

int Setting::load(YAML::Node* conf) {
  YAML::Node dat = (*conf)[key];
  if (!dat.IsDefined()) return 1;
  switch (type) {
    case SETTING_TOGGLE: {
      bool value = dat.as<bool>();
      *(bool*)data = value;
      return 0;
    }
    case SETTING_INT:
    case SETTING_NONE:
    case SETTING_SELECTABLE_INT:
    case SETTING_SELECTABLE_STRING:
    case SETTING_COLOR: {
      int value = dat.as<int>();
      *(int*)data = value;
      return 0;
    }
    case SETTING_FLOAT: {
      float value = dat.as<float>();
      *(float*)data = value;
      return 0;
    }
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

#define S(...) Setting(__VA_ARGS__)
#define C(...) SettingsCategory(__VA_ARGS__)

  settings = {
    C("Audio", {
      /*
        (type),
        (key),(name),(description),
        (bind),
        (ext. data),(ext. data size)
      */
      S(SETTING_INT,
        "channels", "channels", NULL,
        &(params->channels),
        NULL, 0),
      S(SETTING_SELECTABLE_INT,
        "sampleRate", "sample rate", NULL,
        &(params->sampleRate),
        (void*)sampleRateSelectableData, 9),
      S(SETTING_INT,
        "bufferSize", "buffer size", "the number of audio samples to store",
        &(params->audioBufferSize),
        NULL, 0),
      S(SETTING_INT,
        "frameSize", "frame size", NULL,
        &(params->audioFrameSize),
        NULL, 0),
    }),
    C("Colors", {
      S(SETTING_COLOR,
        "chan1col", "channel 1", NULL,
        &(params->chanColor[0]),
        NULL, 0),
      S(SETTING_COLOR,
        "chan2col", "channel 2", NULL,
        &(params->chanColor[1]),
        NULL, 0),
      S(SETTING_COLOR,
        "chan3col", "channel 3", NULL,
        &(params->chanColor[2]),
        NULL, 0),
      S(SETTING_COLOR,
        "trigCol", "successful trigger", NULL,
        &(params->triggeredColor),
        NULL, 0),
      S(SETTING_COLOR,
        "notTrigCol", "unsuccessful trigger", NULL,
        &(params->notTriggeredColor),
        NULL, 0),
      S(SETTING_COLOR,
        "xyCol", "XY scope", "note: the transparency/alpha of this color is the intensity control",
        &(params->xyColor),
        NULL, 0),
    })
  };

#undef S
#undef C

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

YAML::Node USCConfig::getConfig(const char* key) {
  return conf[key];
}

void USCConfig::setConfig(const char* key, bool data) {
  if (conf[key]) {
    conf[key] = data;
  } else {
    conf.force_insert(key, data);
  }
}

void USCConfig::setConfig(const char* key, int data) {
  if (conf[key]) {
    conf[key] = data;
  } else {
    conf.force_insert(key, data);
  }
}

void USCConfig::setConfig(const char* key, float data) {
  if (conf[key]) {
    conf[key] = data;
  } else {
    conf.force_insert(key, data);
  }
}

void USCConfig::setConfig(const char* key, const char* data) {
  if (conf[key]) {
    conf[key] = data;
  } else {
    conf.force_insert(key, data);
  }
}

void USCConfig::saveLayout(const char* layout) {
  if (conf["layout"]) {
    conf["layout"] = layout;
  } else {
    conf.force_insert("layout", layout);
  }
}

std::string USCConfig::getLayout() {
  if (!conf["layout"]) return NULL;
  return conf["layout"].as<std::string>();
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