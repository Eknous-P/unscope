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

#ifndef USC_CONFIG_H
#define USC_CONFIG_H

#include "shared.h"
#include "yaml-cpp/yaml.h"

class USCConfig {
  private:
    const char* file;
    YAML::Node root;

    bool ready;

  public:
    USCConfig();
    USCConfig(const char* confFile);

    YAML::Node& getConfigNode();

    template <typename T>
    int setConfig(const char* key, T v) {
      root[key] = v;
      return 0;
    }
    template <typename T>
    T getConfig(const char* key, T defaultV) {
      if (exists(key)) 
        return root[key].as<T>();
      return defaultV;
    }

    bool exists(const char* key);

    int loadConfig();
    int saveConfig();

    ~USCConfig();
};

template <typename T>
T getConfig(YAML::Node& node, const char* key, T defaultV) {
  if (node[key].IsDefined()) return node[key].as<T>();
  return defaultV;
}

#endif
