#include "gui.h"

void USCGUI::drawSettings() {
  if (!wo.settingsOpen) return;
  ImGui::Begin("Settings", &wo.settingsOpen);
  cf->drawSettings();
  ImGui::End();
}
