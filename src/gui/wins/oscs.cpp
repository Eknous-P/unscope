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
#include <imgui.h>
#include <shared.h>
#include "plotOsc.cpp"

void USCGUI::drawMainScope() {
  if (!wo.mainScopeOpen) return;
  if (!(oscAlign && oscData)) return;
  /*ImGui::Begin("Scope",&wo.mainScopeOpen);
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
  ImGui::End();*/

  ImGui::Begin("Scope", NULL);
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 origin = ImGui::GetWindowPos(), size = ImGui::GetWindowSize();
  float titleBar = ImGui::GetStyle().FramePadding.x*2.f + ImGui::CalcTextSize("Scope").y;
  origin.y += titleBar;
  size.y -= titleBar;
  // v scale labels
  {
    ImVec2 p1, textSize = ImGui::CalcTextSize("-1.000");
    textSize.x += 5.f;
    char buf[16];
    FOR_RANGE(channels) {
      snprintf(buf, 16, " CH.%.2d", z+1);
      p1.y = origin.y + 5.f;
      p1.x = origin.x + textSize.x * z;
      dl->AddText(p1, 0xffffffff, buf);
    }
    FOR_RANGE(9) {
      for (unsigned char c=0; c<channels; c++) {
        p1.y = origin.y + size.y * ((z+1)/10.f) - textSize.y/2;
        p1.x = origin.x + textSize.x * c;
        float v = -((signed char)z-4)/4.f / tc[c].yScale - tc[c].yOffset;
        snprintf(buf, 16, v>=0?" %1.3f":"%1.3f", v);
        dl->AddText(p1, 0xffffffff, buf);
      }
    }
    origin.x += textSize.x * channels;
    size.x -= textSize.x * channels;
  }
  // grid
  {
    ImVec2 p1, p2;
    FOR_RANGE(9) {
      // veritcal lines
      p1.x = origin.x + size.x * ((z+1)/10.f);
      p2.x = p1.x;
      p1.y = origin.y;
      p2.y = p1.y + size.y;
      dl->AddLine(p1, p2, 0x44ffffff);
      // horizontal lines
      p1.y = origin.y + size.y * ((z+1)/10.f);
      p2.y = p1.y;
      p1.x = origin.x;
      p2.x = p1.x + size.x;
      dl->AddLine(p1, p2, 0x44ffffff);
    }
  }

  // waveforms
  // ImVec2 *scaledWave=NULL;
  ImVec2* scaledWave;
  bool* triggered = new bool[channels];
  for (int c = channels-1; c >= 0; c--) {
    if (!tc[c].enable) continue;
    // draw channels in reverse order so channel 1 is always on top
    nint len = tc[c].traceSize;
    long int offset = (tc[c].traceSize / 2.f) * tc[c].xOffset;
    scaledWave = new ImVec2[len];
    nint i=0;
    unsigned char trigChan = (shareTrigger>0)?(shareTrigger-1):(shareParams?0:c);
    if (shareTrigger>0) {
      if (trigChan == c) {
        memset(triggered, trigger[trigChan]->trigger(tc[trigChan].traceSize), channels);
      }
    }
    else triggered[c] = trigger[c]->trigger(tc[trigChan].traceSize);
    for (; i < len; i++) {
      scaledWave[i].x = origin.x + size.x*((float)i/(float)len);

      nint cur = i;
      if (triggered[c]) {
        cur += trigger[trigChan]->getTriggerIndex();
      }
      else if (doFallback) cur += oscDataSize - tc[c].traceSize;
      else continue;

      cur-=offset;

      if (cur > oscDataSize) break;
      scaledWave[i].y = origin.y - (oscData[c][cur] * tc[c].yScale + tc[c].yOffset - 1.f) * size.y/2.f;
    }
    dl->AddPolyline(scaledWave, i, ImGui::ColorConvertFloat4ToU32(tc[c].color), 0, .4f);
    delete[] scaledWave;
  }
  // drags
  {
    FOR_RANGE(channels) {
      ImVec4 trigColor = trigger[z]->getTriggered()?ImVec4(0,1,0,.5f):ImVec4(1,0,0,.5f);
      plotDragX((float*)&tc[z].xOffset, "test", dl, ImVec4(origin.x, origin.y, size.x, size.y));
      for (TriggerParam p : trigger[z]->getParams()) {
        if (shareTrigger > 0 && z != shareTrigger - 1) continue;
        if (p.bindToDragX) {
          plotDragX((float*)p.getValuePtr(), "test", dl, ImVec4(origin.x, origin.y, size.x, size.y));
        }
        if (p.bindToDragY) {
          // if (ImPlot::DragLineY(2*i+1, &valD, trigColor)) p.setValue<float>(valD);
          // ImPlot::TagY(valD, trigColor, "CH %d", i + 1);
        }
      }
    }
  }

  ImGui::End();
}

bool USCGUI::plotDragX(float* v, const char* label, ImDrawList* dl, ImVec4 rect, float v_min, float v_max) {
  ImVec2 p1, p2;
  p1.x = rect.x + (*v + 1.f) * rect.z/2.f;
  p2.x = p1.x;
  p1.y = rect.y;
  p2.y = p1.y + rect.w;
  dl->AddLine(p1, p2, 0xff0000ff);
  ImGui::SetCursorPosX(p1.x-2.5f - rect.x);
  ImGui::Dummy(ImVec2(5.f, rect.w));
  if (ImGui::IsItemHovered()) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
  if (ImGui::IsItemClicked()) {
    float mouseX = ImGui::GetMousePos().x;
    *v = mapToRange(ImVec2(rect.z,rect.w), ImVec2(v_min,v_max), mouseX-rect.x);
    if (*v < v_min) *v = v_min;
    if (*v > v_max) *v = v_max;
    return true;
  }
  }
  return false;
}

void USCGUI::drawXYScope() {
  if (!oscData) return;
  if (!wo.xyScopeOpen) return;
  if (channels < 2) return;
  ImGui::Begin("Scope (XY)",&wo.xyScopeOpen);
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 origin = ImGui::GetWindowPos(), size = ImGui::GetWindowSize();
  float titleBar = ImGui::GetStyle().FramePadding.x*2.f + ImGui::CalcTextSize("Scope (XY)").y;
  origin.y += titleBar;
  size.y -= titleBar;
  ImVec2 sizeHalf = size/2;
  float sizeMin = size.x<size.y?size.x:size.y;
  // grid
  {
    ImVec2 p1, p2;
    p1.x = origin.x + sizeHalf.x - sizeMin/2.f;
    p1.y = origin.y + sizeHalf.y - sizeMin/2.f;
    p2.x = origin.x + sizeHalf.x + sizeMin/2.f;
    p2.y = origin.y + sizeHalf.y + sizeMin/2.f;
    dl->AddRect(p1, p2, 0x44ffffff);
    FOR_RANGE(9) {
      // veritcal lines
      p1.x = origin.x + sizeHalf.x - sizeMin * ((z-4)/10.f);
      p2.x = p1.x;
      p1.y = origin.y + sizeHalf.y - sizeMin/2.f;
      p2.y = origin.y + sizeHalf.y + sizeMin/2.f;
      dl->AddLine(p1, p2, 0x44ffffff);
      // horizontal lines
      p1.y = origin.y + sizeHalf.y - sizeMin * ((z-4)/10.f);
      p2.y = p1.y;
      p1.x = origin.x + sizeHalf.x - sizeMin/2.f;
      p2.x = origin.x + sizeHalf.x + sizeMin/2.f;
      dl->AddLine(p1, p2, 0x44ffffff);
    }
  }
  sizeMin/=2.f;
  ImVec2* scaledPlot = new ImVec2[xyp.sampleLen];
  for (nint i=0; i<xyp.sampleLen; i++) {
    // scaledWave[i].y = origin.y - (oscData[c][cur] * tc[c].yScale + tc[c].yOffset - 1.f) * size.y/2.f;
    nint cur = oscDataSize - xyp.sampleLen + i;
    scaledPlot[i].x = origin.x + (oscData[xyp.axisChan[0]-1][cur] * xyp.xScale + xyp.xOffset) * sizeMin + sizeHalf.x;
    scaledPlot[i].y = origin.y - (oscData[xyp.axisChan[1]-1][cur] * xyp.yScale + xyp.yOffset) * sizeMin + sizeHalf.y;
  }
  dl->AddPolyline(scaledPlot, xyp.sampleLen, ImGui::ColorConvertFloat4ToU32(xyp.color), 0, 1.0f);
  delete[] scaledPlot;
  ImGui::End();
}
