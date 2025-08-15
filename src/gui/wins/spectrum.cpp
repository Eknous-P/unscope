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
#include <fftw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <shared.h>

void USCGUI::drawSpectrum(bool* open) {
  if (!oscData) return;
  if (!*open) return;
  if (channels < 2) return;
  ImGui::Begin("Spectrum",open);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 origin = ImGui::GetWindowPos(), size = ImGui::GetWindowSize();
  float titleBar = ImGui::GetCurrentWindow()->TitleBarHeight;
  origin.y += titleBar;
  size.y -= titleBar;
  ImVec2 sizeHalf = size/2;
  float sizeMin = size.x<size.y?size.x:size.y;
  // grid
  // {
  //   ImVec2 p1, p2;
  //   p1.x = origin.x + sizeHalf.x - sizeMin/2.f;
  //   p1.y = origin.y + sizeHalf.y - sizeMin/2.f;
  //   p2.x = origin.x + sizeHalf.x + sizeMin/2.f;
  //   p2.y = origin.y + sizeHalf.y + sizeMin/2.f;
  //   dl->AddRect(p1, p2, 0x44ffffff);
  //   FOR_RANGE(9) {
  //     // veritcal lines
  //     p1.x = origin.x + sizeHalf.x - sizeMin * ((z-4)/10.f);
  //     p2.x = p1.x;
  //     p1.y = origin.y + sizeHalf.y - sizeMin/2.f;
  //     p2.y = origin.y + sizeHalf.y + sizeMin/2.f;
  //     dl->AddLine(p1, p2, 0x44ffffff);
  //     // horizontal lines
  //     p1.y = origin.y + sizeHalf.y - sizeMin * ((z-4)/10.f);
  //     p2.y = p1.y;
  //     p1.x = origin.x + sizeHalf.x - sizeMin/2.f;
  //     p2.x = origin.x + sizeHalf.x + sizeMin/2.f;
  //     dl->AddLine(p1, p2, 0x44ffffff);
  //   }
  // }
  sizeMin/=2.f;
  FOR_RANGE(channels) {
    unsigned int samples = sd[z].samples;
    if (sd[z].updatePlan) {
      sd[z].updatePlan=false;
      if (sd[z].in)  delete[] sd[z].in;
      if (sd[z].out) fftw_free(sd[z].out);
      if (sd[z].p)   fftw_destroy_plan(sd[z].p);
      sd[z].in = new double[samples*2];
      sd[z].out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*samples*2);
      sd[z].p = fftw_plan_dft_r2c_1d(samples*2, sd[z].in, sd[z].out, FFTW_ESTIMATE);
    }
    nint cur=oscDataSize-samples*2;
    for (unsigned int i = 0; i < samples*2; i++) {
      float f = oscData[z][cur+i];
      sd[z].in[i] = f * 0.5 * (1 - cos(2*M_PI*i/samples));
    }
    fftw_execute(sd[z].p);
    ImVec2* scaledPlot = new ImVec2[samples];
    for (unsigned int i=0; i<samples; i++) {
      // switch (sd[z].scale&0xf) {
      //   case 1:
          scaledPlot[i].x = origin.x + size.x*log2((float)i/samples+1.f);
      //   case 0:
      //   default:
      //     scaledPlot[i].x = origin.x + (size.x/samples)*i;
      // }
      scaledPlot[i].y = origin.y + size.y - size.y*(sqrt(sd[z].out[i][0] * sd[z].out[i][0] + sd[z].out[i][1] * sd[z].out[i][1]));
    }
    dl->PushClipRectFullScreen();
    dl->AddPolyline(scaledPlot, samples, ImGui::ColorConvertFloat4ToU32(sd[z].color), 0, 1.0f);
    dl->PopClipRect();
    delete[] scaledPlot;
  }
  ImGui::PopStyleVar();
  ImGui::End();
  // ImGui::Begin("fft data");
  // if (ImGui::BeginTable("data", 4)) {
  //     ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
  //     ImGui::TableNextColumn();
  //     ImGui::Text("i");
  //     ImGui::TableNextColumn();
  //     ImGui::Text("input");
  //     ImGui::TableNextColumn();
  //     ImGui::Text("real");
  //     ImGui::TableNextColumn();
  //     ImGui::Text("complex");
  //   for (int i=0; i<sd[0].samples; i++) {
  //     ImGui::TableNextRow();
  //     ImGui::TableNextColumn();
  //     ImGui::Text("%d",i);
  //     ImGui::TableNextColumn();
  //     ImGui::Text("%f",sd[0].in[i]);
  //     ImGui::TableNextColumn();
  //     ImGui::Text("%f",sd[0].out[i][0]);
  //     ImGui::TableNextColumn();
  //     ImGui::Text("%f",sd[0].out[i][1]);
  //   }
  //   ImGui::EndTable();
  // }
  // ImGui::End();
}
