#include "gui.h"
#include "imgui_toggle.h"

void USCGUI::drawCursors() {
  if (!wo.cursorsOpen) return;
  ImGui::Begin("Cursors",&wo.cursorsOpen);
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