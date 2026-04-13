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

#include "gui.h"

USCGUI::SettingsCategory::SettingsCategory() {
  name = NULL;
  settings = {};
  children = {};
}

USCGUI::SettingsCategory::SettingsCategory(const char* n, std::initializer_list<Parameter> p, std::initializer_list<SettingsCategory> c) {
  name = n;
  settings = p;
  children = c;
}

void USCGUI::SettingsCategory::drawParams() {
  // empty settings means a root node for the category
  // root node doesnt need indenting
  if (name) ImGui::SeparatorText(name);
  for (Parameter& p:settings)
    p.draw();
  if (!children.empty()) {
    if (name) ImGui::Indent();
      for (SettingsCategory& c:children)
        c.drawParams();
    if (name) ImGui::Unindent();
  }
}

void USCGUI::SettingsCategory::readConfig(USCConfig* conf) {
  for (Parameter& p:settings)
    p.readFromConfig(conf);
  for (SettingsCategory& c:children)
    c.readConfig(conf);
}

void USCGUI::SettingsCategory::writeConfig(USCConfig* conf) {
  for (Parameter& p:settings)
    p.writeToConfig(conf);
  for (SettingsCategory& c:children)
    c.writeConfig(conf);
}

void USCGUI::drawSettings(bool* open) {
  if (!*open) return;
  if (ImGui::Begin("Settings", open)) {
    settingsParams.drawParams();
    if (ImGui::Button("reset")) readConfig();
    ImGui::SameLine();
    if (ImGui::Button("apply")) writeConfig();
    ImGui::SameLine();
    if (ImGui::Button("OK")) {
      writeConfig();
      *open = false;
    }
  }
  ImGui::End();
}

