/*
unscope - an audio oscilloscope
Copyright (C) 2025 Eknous

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
#include "imgui_toggle.h"

void USCGUI::drawCursors(bool* open) {
  if (!*open) return;
  ImGui::Begin("Cursors",open);
  ImGui::Toggle("show horizontal cursors", &showHCursors);
  ImGui::Toggle("show vertical cursors", &showVCursors);

  if (showHCursors || showVCursors) {
    FOR_RANGE(channels) {
      ImGui::Text("Channel %d", z + 1);
      if (showHCursors) {
        const float tDiff = fabsf(
          (tc[z].timebase) * ((HCursors[1].pos - HCursors[0].pos) / 2.0f)
        );
        ImGui::Text("X1: %.3f, X2: %.3f\ntime:%2.4fms (%4.4fHz)",
          HCursors[0].pos,
          HCursors[1].pos,
          tDiff,
          1000.0f / tDiff
        );
      }
      if (showVCursors) {
        const float vDiff = fabsf(VCursors[1].pos - VCursors[0].pos); // TODO: v cal
        ImGui::Text("Y1: %.3f, Y2: %.3f\nvoltage (p-p):%2.4fV",
          VCursors[0].pos,
          VCursors[1].pos,
          vDiff
        );
      }
      ImGui::Separator();
    }
  }
  ImGui::End();
}
