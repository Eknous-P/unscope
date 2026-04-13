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

#include "shared.h"
#include "gui.h"
#include "imgui-knobs.h"
#include "imgui_toggle.h"

void USCGUI::drawChanControls() {
  if (settings.scopeContainChannelControls) return;
  char strbuf[64];
  for (int j=0; j<scopeWindows.size(); j++) {
    vector<ScopeChannel*>* chans = &scopeWindows[j].channels;
    for (int i=0; i<chans->size(); i++) {
      snprintf(strbuf, 64, "Channel %d controls", i+1);
      if (!chans->at(i)->showControls) continue;
      if (ImGui::Begin(strbuf, &chans->at(i)->showControls)) {
        chans->at(i)->drawParams();
      }
      ImGui::End();
    }
  }
}
