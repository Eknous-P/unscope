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
#include <imgui_internal.h>

bool USCGUI::plotDragX(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min, float v_max) {
  ImVec2 p1, p2;
  p1.x = rect.x + (*v + 1.f) * rect.z/2.f;
  p2.x = p1.x;
  p1.y = rect.y;
  p2.y = p1.y + rect.w;
  dl->AddLine(p1, p2, col);
  ImGui::SetCursorPosX(p1.x-2.5f);
  ImGui::SetCursorPosY(0.0f);
  ImGui::PushID(label);
  ImGui::InvisibleButton(label, ImVec2(5.f,rect.w));
  float min=v_min, max=v_max;
  bool ret = ImGui::DragBehavior(ImGui::GetID(label), ImGuiDataType_Float, v, 2.f/rect.z, &min, &max, "", ImGuiSliderFlags_None);
  if (ImGui::IsItemHovered() || ret) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary|ImGuiHoveredFlags_DelayNormal)) {
    if (ImGui::BeginTooltip()) {
      ImGui::Text("%s", label);
      ImGui::EndTooltip();
    }
  }
  ImGui::PopID();
  return ret;
}

bool USCGUI::plotDragY(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min, float v_max) {
  ImGuiWindow* window = ImGui::GetCurrentWindow();
  ImVec2 p1, p2;
  p1.x = rect.x;
  p2.x = p1.x + rect.z;
  p1.y = rect.y + (-*v + 1.f) * rect.w/2.f;
  p2.y = p1.y;
  dl->AddLine(p1, p2, col);
  window->DC.CursorPos=ImVec2(p1.x,p1.y-2.5f)-window->Scroll;
  ImGui::PushID(label);
  ImGui::InvisibleButton(label, ImVec2(rect.z, 5.f));
  float min=v_min, max=v_max;
  bool ret = ImGui::DragBehavior(ImGui::GetID(label), ImGuiDataType_Float, v, 2.f/rect.w, &min, &max, "", ImGuiSliderFlags_Vertical);
  if (ImGui::IsItemHovered() || ret) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary|ImGuiHoveredFlags_DelayNormal)) {
    if (ImGui::BeginTooltip()) {
      ImGui::Text("%s", label);
      ImGui::EndTooltip();
    }
  }
  ImGui::PopID();
  return ret;
}

void USCGUI::drawMainScope() {
  if (!wo.mainScopeOpen) return;
  if (!oscData) return;

  ImGui::Begin("Scope", NULL);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 origin = ImGui::GetWindowPos(), size = ImGui::GetWindowSize();
  float titleBar = ImGui::GetCurrentWindow()->TitleBarHeight;
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
      dl->AddText(p1, ImGui::GetColorU32(ImGuiCol_Text), buf);
    }
    FOR_RANGE(9) {
      for (unsigned char c=0; c<channels; c++) {
        p1.y = origin.y + size.y * ((z+1)/10.f) - textSize.y/2;
        p1.x = origin.x + textSize.x * c;
        float v = -((signed char)z-4)/4.f / tc[c].yScale - tc[c].yOffset;
        snprintf(buf, 16, v>=0?" %1.3f":"%1.3f", v);
        dl->AddText(p1, ImGui::GetColorU32(ImGuiCol_Text), buf);
      }
    }
    origin.x += textSize.x * channels;
    size.x   -= textSize.x * channels;
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
  FOR_RANGE(channels) {
    unsigned char trigChan = (shareTrigger>0)?(shareTrigger-1):(shareParams?0:z);
    if (shareTrigger>0) {
      if (trigChan == z) {
        memset(triggered, trigger[trigChan]->trigger(tc[trigChan].traceSize), channels);
      }
    }
    else triggered[z] = trigger[z]->trigger(tc[trigChan].traceSize);
  }
  for (int c = channels-1; c >= 0; c--) {
    if (!tc[c].enable) continue;
    if (tc[c].color.w==0.0f) continue;
    // draw channels in reverse order so channel 1 is always on top
    nint len = tc[c].traceSize;
    long int offset = (tc[c].traceSize / 2.f) * tc[c].xOffset;
    scaledWave = new ImVec2[len];
    nint i=0;
    unsigned char trigChan = (shareTrigger>0)?(shareTrigger-1):(shareParams?0:c);
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
  delete[] triggered;
  // drags
  {
    char buf[256];
    FOR_RANGE(channels) {
      ImVec4 trigColor = trigger[z]->getTriggered()?ImVec4(0,1,0,.5f):ImVec4(1,0,0,.5f);
      snprintf(buf, 256, "x offset (ch %d)", z+1);
      plotDragX(&tc[z].xOffset, buf, dl, ImVec4(origin.x, origin.y, size.x, size.y), 0xff00ff33);
      // snprintf(buf, 256, "y offset##CH%d", z);
      // plotDragY(&tc[z].yOffset, buf, dl, ImVec4(origin.x, origin.y, size.x, size.y), 0xff00ff33);
      for (TriggerParam p : trigger[z]->getParams()) {
        if (shareTrigger > 0 && z != shareTrigger - 1) continue;
        if (p.bindToDragX) {
          snprintf(buf, 256, "%s (ch %d)", p.getLabel(), z+1);
          plotDragX((float*)p.getValuePtr(), buf, dl, ImVec4(origin.x, origin.y, size.x, size.y), 0xff0077ff);
        }
        if (p.bindToDragY) {
          snprintf(buf, 256, "%s (ch %d)", p.getLabel(), z+1);
          plotDragY((float*)p.getValuePtr(), buf, dl, ImVec4(origin.x, origin.y-tc[z].yOffset*size.y/2.0f, size.x, size.y), 0xff0077ff);
        }
      }
    }
  }
  // cursors (also drags)
  if (showHCursors) {
    ImVec4 cursorColor = ImVec4(.24f, .13f, .98f, 1.f);
    char buf[256];
    plotDragX(&HCursors[0].pos, "x cursor 1", dl, ImVec4(origin.x, origin.y, size.x, size.y), ImGui::ColorConvertFloat4ToU32(cursorColor));
    plotDragX(&HCursors[1].pos, "x cursor 2", dl, ImVec4(origin.x, origin.y, size.x, size.y), ImGui::ColorConvertFloat4ToU32(cursorColor));

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
      dl->AddText(origin+ImVec2(5.0f, 5.0f+10.0f*z),ImGui::GetColorU32(ImGuiCol_Text), buf);
    }
  }
  if (showVCursors) {
    ImVec4 cursorColor = ImVec4(.94f, .73f, .18f, 1.f);
    char buf[256];
    plotDragY(&VCursors[0].pos, "y cursor 1", dl, ImVec4(origin.x, origin.y, size.x, size.y), ImGui::ColorConvertFloat4ToU32(cursorColor));
    plotDragY(&VCursors[1].pos, "y cursor 2", dl, ImVec4(origin.x, origin.y, size.x, size.y), ImGui::ColorConvertFloat4ToU32(cursorColor));

    FOR_RANGE(channels) {
      const float vDiff = fabsf(VCursors[1].pos - VCursors[0].pos); // TODO: v cal
      snprintf(buf, 256, "[CH %d] Y1: %.3f Y2: %.3f | %2.4fV p-p",
        z + 1,
        VCursors[0].pos,
        VCursors[1].pos,
        vDiff
      );
      dl->AddText(origin+ImVec2(5.0f, size.y-15.0f-10.0f*z),ImGui::GetColorU32(ImGuiCol_Text), buf);
    }
  }
  // trig hints
  {
    if (trigNum==TRIG_SMOOTH) {
      FOR_RANGE(channels) {
        if (!tc[z].enable) continue;
        if (shareTrigger>0) {
          if (z!=shareTrigger-1) continue;
        }
        if (trigger[z]->getParams()[0].isHovered()) {
          nint len = tc[z].traceSize;
          long int offset = (tc[z].traceSize / 2.f) * tc[z].xOffset;
          scaledWave = new ImVec2[len];
          nint i=0;
          unsigned char trigChan = (shareTrigger>0)?(shareTrigger-1):(shareParams?0:z);
          float* smoothBuf=((TriggerSmooth*)(trigger[z]))->getSmoothBuffer();
          for (; i < len; i++) {
            scaledWave[i].x = origin.x + size.x*((float)i/(float)len);

            nint cur = i;
            if (triggered[z]) {
              cur += trigger[trigChan]->getTriggerIndex();
            }
            else if (doFallback) cur += oscDataSize - tc[z].traceSize;
            else continue;

            cur-=offset;

            if (cur > oscDataSize) break;
            scaledWave[i].y = origin.y - (smoothBuf[cur] * tc[z].yScale * (1.0f+trigger[z]->getParams()[0].getValue<float>()) + tc[z].yOffset - 1.f) * size.y/2.f;
          }
          dl->AddPolyline(scaledWave, i, 0xffff66ff, 0, .4f);
          delete[] scaledWave;
        }
        if (trigger[z]->getParams()[1].isHovered()) {
          float level=((TriggerSmooth*)(trigger[z]))->getTriggerLevel();
          level=origin.y+(level+1.0f)*size.y/2.0f;
          dl->AddLine(
            ImVec2(origin.x,level), 
            ImVec2(origin.x+size.x, level),
            0xff0077ff
          );
        }
      }
    }
  }
  ImGui::PopStyleVar();
  ImGui::End();
}

void USCGUI::drawXYScope() {
  if (!oscData) return;
  if (!wo.xyScopeOpen) return;
  if (channels < 2) return;
  ImGui::Begin("Scope (XY)",&wo.xyScopeOpen);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 origin = ImGui::GetWindowPos(), size = ImGui::GetWindowSize();
  float titleBar = ImGui::GetCurrentWindow()->TitleBarHeight;
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
  ImGui::PopStyleVar();
  ImGui::End();
}
