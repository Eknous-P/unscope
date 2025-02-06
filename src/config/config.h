#ifndef CONFIG_H
#define CONFIG_H

#include "shared.h"
#include "yaml.h"
#include "imgui.h"

// looks familiar?

enum SettingTypes : unsigned char {
  SETTING_NONE = 0,
  SETTING_TOGGLE,
  SETTING_INT,
  SETTING_FLOAT,
  SETTING_STRING
};

class Setting {
  void* data;
  SettingTypes type;
  const char *key, *label, *desc;
  public:
    bool passesFilter(ImGuiTextFilter* filter);
    void draw();
    int save(YAML::Node* conf);
    int load(YAML::Node* conf);
    Setting(SettingTypes type, const char* k, const char* l, const char* d);
    ~Setting();
};

// 

class USCConfig {
  YAML::Node conf;
  std::vector<Setting> settings;

  public:
    int loadConfig();
    void drawSettings();
    USCConfig(const char* filePath);
    ~USCConfig();
};

#endif