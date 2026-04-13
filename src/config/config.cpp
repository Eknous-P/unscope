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

#include "config.h"
#include "shared.h"
#include <fstream>

USCConfig::USCConfig():
  file(NULL),
  root(),
  ready(false) {}

USCConfig::USCConfig(const char* confFile) {
  file = confFile;
  ready = true;
}

YAML::Node& USCConfig::getConfigNode() {
  return root;
}

bool USCConfig::exists(const char* key) {
  return root[key].IsDefined();
}

int USCConfig::loadConfig() {
  if (!ready) return -1;
  try {
    root = YAML::LoadFile(file);
  } catch (YAML::ParserException) {
    printf(ERROR_MSG "config: malformed yaml file!" MSG_END);
    return 1;
  } catch (YAML::BadFile) {
    printf(ERROR_MSG "config: bad file!" MSG_END);
    printf(INFO_MSG "new file will be made..." MSG_END);
    root["configSet"] = 1;
    return 1;
  }
  if (root.IsDefined()) {
    try {
      printf(INFO_MSG "config: config set: %s" MSG_END, root["configSet"].as<int>()?"true":"false");
    } catch (YAML::BadConversion) {
      printf(ERROR_MSG "config: what" MSG_END);
      root["configSet"] = 1;
    }
    return 0;
  }
  return 1;
}

int USCConfig::saveConfig() {
  printf(INFO_MSG "config: writing to %s" MSG_END, file);
  std::ofstream cfile(file);
  cfile << root;
  cfile.close();
  return 0;
}

USCConfig::~USCConfig() {
}
