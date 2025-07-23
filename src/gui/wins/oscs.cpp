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
#include <implot.h>

void USCGUI::drawMainScope() {
  if (!wo.mainScopeOpen) return;
  if (!(oscAlign && oscData)) return;
  ImGui::Begin("Scope",&wo.mainScopeOpen);
  if (ImPlot::BeginPlot("##scope", ImGui::GetContentRegionAvail(),sc.plotFlags)) {
    for (unsigned char i = 0; i < channels; i++) {
      ImPlot::SetupAxis(ImAxis(i),"t",ImPlotAxisFlags_NoLabel|ImPlotAxisFlags_NoTickLabels|sc.scopeFlags);
      ImPlot::SetupAxis(ImAxis(i+ImAxis_Y1),"v",sc.scopeFlags|ImPlotAxisFlags_NoLabel);
      ImPlot::SetupAxisLimits(ImAxis(i),-1.0f, 1.0f);
      ImPlot::SetupAxisLimits(ImAxis(i+ImAxis_Y1),-1.0f/tc[i].yScale-tc[i].yOffset,1.0f/tc[i].yScale-tc[i].yOffset);
    }
    for (unsigned char i = 0; i < channels; i++) {
      if (!tc[i].enable) continue;
      ImPlot::SetAxes(i,i+ImAxis_Y1);
      ImPlot::SetNextLineStyle(tc[i].color,0.25f);
      unsigned char trigChan = (shareTrigger>0)?(shareTrigger-1):i;
      if (oscAlign[i] && oscData[i] && oscDataSize) { // TODO: handle OOB when rewriting this
        ImPlot::PlotLine("##scopeplot", oscAlign[trigChan], oscData[i], oscDataSize,ImPlotFlags_NoLegend);
      }
      ImVec4 trigColor = trigger[i]->getTriggered()?ImVec4(0,1,0,.5f):ImVec4(1,0,0,.5f);
      for (TriggerParam p : trigger[i]->getParams()) {
        if (shareTrigger > 0 && i != shareTrigger - 1) continue;
        double valD = p.getValue<float>();
        if (p.bindToDragX) {
          if (ImPlot::DragLineX(2*i+1, &valD, tc[i].color)) p.setValue<float>(valD);
          // ImPlot::TagX(valD, trigColor, "CH %d", i + 1);
        }
        if (p.bindToDragY) {
          if (ImPlot::DragLineY(2*i+1, &valD, trigColor)) p.setValue<float>(valD);
          ImPlot::TagY(valD, trigColor, "CH %d", i + 1);
        }
      }

      if (showHCursors) {
        ImVec4 cursorColor = ImVec4(.24f, .13f, .98f, 1.f);
        char buf[256];
        ImGui::PushID("HCURSOR0");
        if (ImPlot::DragLineX(ImAxis_X1, &HCursors[0].pos, cursorColor)) {
          HCursors[0].pos = clamp(HCursors[0].pos);
        }
        ImPlot::SetAxis(ImAxis_X1);
        ImPlot::TagX(HCursors[0].pos, cursorColor, "%s", HCursors[0].label);
        ImGui::PopID();
        ImGui::PushID("HCURSOR1");
        if (ImPlot::DragLineX(ImAxis_X2, &HCursors[1].pos, cursorColor)) {
          HCursors[1].pos = clamp(HCursors[1].pos);
        }
        ImPlot::SetAxis(ImAxis_X1);
        ImPlot::TagX(HCursors[1].pos, cursorColor, "%s", HCursors[1].label);

        FOR_RANGE(channels) {
          const float tDiff = fabsf(
            (tc[z].timebase) * ((HCursors[1].pos - HCursors[0].pos) / 2.0f)
          );
          snprintf(buf, 256, "[CH %d] X1: %.3f X2: %.3f | %2.4fms (%4.4fHz)",
            z + 1,
            HCursors[0].pos,
            HCursors[1].pos,
            tDiff,
            1000.0f / tDiff
          );
          ImPlot::PlotText(buf, -0.55f, 0.9f - z / 8.f);
        }
        ImGui::PopID();
      }
      if (showVCursors) {
        ImVec4 cursorColor = ImVec4(.94f, .73f, .18f, 1.f);
        char buf[256];
        ImGui::PushID("VCURSOR0");
        if (ImPlot::DragLineY(ImAxis_Y1, &VCursors[0].pos, cursorColor)) {
          VCursors[0].pos = clamp(VCursors[0].pos);
        }
        // ImPlot::SetAxis(ImAxis_Y1);
        ImPlot::TagY(VCursors[0].pos, cursorColor, "%s", VCursors[0].label);
        ImGui::PopID();
        ImGui::PushID("VCURSOR1");
        // ImPlot::SetAxis(ImAxis_Y2);
        if (ImPlot::DragLineY(ImAxis_Y2, &VCursors[1].pos, cursorColor)) {
          VCursors[1].pos = clamp(VCursors[1].pos);
        }
        // ImPlot::SetAxis(ImAxis_Y1);
        ImPlot::TagY(VCursors[1].pos, cursorColor, "%s", VCursors[1].label);
        ImGui::PopID();

        FOR_RANGE(channels) {
          const float vDiff = fabsf(VCursors[1].pos - VCursors[0].pos); // TODO: v cal
          snprintf(buf, 256, "[CH %d] Y1: %.3f Y2: %.3f | %2.4fV p-p",
            z + 1,
            VCursors[0].pos,
            VCursors[1].pos,
            vDiff
          );
          ImPlot::PlotText(buf, -0.65f, -0.7f - z / 8.f);
        }
      }

    }
    ImPlot::EndPlot();
  }
  ImGui::End();
}

void USCGUI::drawXYScope() {
  if (!oscData) return;
  if (!wo.xyScopeOpen) return;
  if (channels < 2) return;
  ImGui::Begin("Scope (XY)",&wo.xyScopeOpen);
  if (ImPlot::BeginPlot("##scopexy", ImGui::GetContentRegionAvail(),sc.plotFlags|ImPlotFlags_Equal)) {
    ImPlot::SetupAxes("##x","##y",ImPlotAxisFlags_NoTickLabels|ImPlotAxisFlags_Lock,sc.scopeFlags|ImPlotAxisFlags_NoTickLabels);
    ImPlot::SetupAxisLimits(ImAxis_X1,-1.0f/xyp.xScale-xyp.xOffset,1.0f/xyp.xScale-xyp.xOffset);
    ImPlot::SetupAxisLimits(ImAxis_Y1,-1.0f/xyp.yScale-xyp.yOffset,1.0f/xyp.yScale-xyp.yOffset);
    ImPlot::SetNextLineStyle(xyp.color,0.125f);
    ImPlot::PlotLine("##scopeplot", oscData[xyp.axisChan[0]-1] + (oscDataSize - xyp.sampleLen), oscData[xyp.axisChan[1]-1] + (oscDataSize - xyp.sampleLen), xyp.sampleLen,ImPlotFlags_NoLegend);
    ImPlot::EndPlot();
  }
  ImGui::End();
}
