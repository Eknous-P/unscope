#ifndef CONFIG_H
#define CONFIG_H

#include "shared.h"
#include "yaml.h"
#include "imgui.h"

// looks familiar? (https://github.com/Eknous-P/furnace/tree/settings3)

enum SettingTypes : unsigned char {
  SETTING_NONE = 0,
  SETTING_TOGGLE,
  SETTING_INT,
  SETTING_FLOAT,
  SETTING_STRING,
  SETTING_SELECTABLE_INT,
  SETTING_SELECTABLE_STRING,
  SETTING_COLOR,
};

class Setting {
  void *data, *extData;
  size_t extDataSize;
  SettingTypes type;
  const char *key, *label, *desc;
  bool isBound;
  public:
    bool passesFilter(ImGuiTextFilter* filter);
    void draw();
    void *getData();
    int save(YAML::Node* conf);
    int load(YAML::Node* conf);
    void destroy();
    Setting(SettingTypes t, const char* k, const char* l, const char* d, void* bind, void* ext, size_t es);
};

struct SettingsCategory {
  const char* name;
  std::vector<Setting> settings;
  // handle recursion later, not needed now
  SettingsCategory(const char* n, std::initializer_list<Setting> s) {
    name = n;
    settings = s;
  }
};

// 

class USCConfig {
  YAML::Node conf;
  bool isEmpty;
  const char* confFile;

  public:
    std::vector<SettingsCategory> settings;
    int loadConfig();
    int saveConfig();
    void drawSettings();
    USCConfig(const char* filePath, unscopeParams* p);
    ~USCConfig();
};

#endif