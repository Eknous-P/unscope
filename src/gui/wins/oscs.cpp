#include "gui.h"

void GUI::drawMainScope() {
  ImGui::Begin("Scope");
  if (ImPlot::BeginPlot("##scope", ImGui::GetContentRegionAvail(),sc.plotFlags)) {
    for (unsigned char i = 0; i < channels; i++) {
      ImPlot::SetupAxis(ImAxis(i),"t",ImPlotAxisFlags_NoDecorations|sc.scopeFlags);
      ImPlot::SetupAxis(ImAxis(i+3),"v",ImPlotAxisFlags_NoDecorations|sc.scopeFlags);
      ImPlot::SetupAxisLimits(ImAxis(i),-1.0f, 1.0f);
      ImPlot::SetupAxisLimits(ImAxis(i+3),-1.0f/tc[i].yScale+tc[i].yOffset,1.0f/tc[i].yScale+tc[i].yOffset);
    }
      ImPlot::SetupAxis(ImAxis_Y1,"##v",sc.scopeFlags);
    for (unsigned char i = 0; i < channels; i++) {
      ImPlot::SetNextLineStyle(ImVec4(tc[i].color[0],tc[i].color[1],tc[i].color[2],tc[i].color[3]),0.25f);
      if (oscAlign[i] && oscData[i] && oscDataSize)
        ImPlot::PlotLine("##scopeplot", oscAlign, oscData[i], oscDataSize,ImPlotFlags_NoLegend);
      ImVec4 trigColor = ai->didTrigger()?ImVec4(0,1,0,.5f):ImVec4(1,0,0,.5f);
      if (showTrigger) ImPlot::TagY(tc[i].trigger,trigColor,"trig");
      double trigDouble = tc[i].trigger;
      double offsDouble = tc[i].trigOffset;
      if (ImPlot::DragLineY(2*i,&trigDouble,trigColor)) tc[i].trigger = trigDouble;
      if (ImPlot::DragLineX(2*i+1,&offsDouble,ImVec4(0,1,0,.5))) {
        tc[i].trigOffset = clamp(offsDouble);
        tc[i].traceOffset = ((tc[i].trigOffset + 1.0f)/2) * tc[i].traceSize;
        if (tc[i].traceSize != 0) {
          tc[i].trigOffset = 2*((float)tc[i].traceOffset/(float)tc[i].traceSize)-1.0f;
        } else {
          tc[i].trigOffset = 0;
        }
      }
    }
    ImPlot::EndPlot();
  }
  ImGui::End();
}

void GUI::drawAuxScope() {
  // ImGui::Begin("Scope (Auxiliary)");
  // if (ImPlot::BeginPlot("##scopeaux", ImGui::GetContentRegionAvail(),sc.plotFlags)) {
  //   ImPlot::SetupAxes("t","##v",sc.scopeFlags,0);
  //   ImPlot::SetupAxisLimits(ImAxis_X1,(float)(oscDataSize - sc.traceSize)/oscDataSize, 1);
  //   ImPlot::SetupAxisLimits(ImAxis_Y1,-1.0f/sc.yScale,1.0f/sc.yScale);
  //   ImPlot::PlotLine("##scopeplot", oscAlign, oscAuxData[0], oscDataSize,ImPlotFlags_NoLegend, 0);
  //   ImPlot::EndPlot();
  // }
  // ImGui::End();
}

void GUI::drawXYScope() {
  if (channels < 2) return;
  ImGui::Begin("Scope (XY)");
  if (ImPlot::BeginPlot("##scopexy", ImGui::GetContentRegionAvail(),sc.plotFlags)) {
    ImPlot::SetupAxes("##x","##y",ImPlotAxisFlags_AutoFit,sc.scopeFlags);
    ImPlot::SetupAxisLimits(ImAxis_X1,-1.0f,1.0f);
    ImPlot::SetupAxisLimits(ImAxis_Y1,-1.0f,1.0f);
    ImPlot::SetNextLineStyle(ImVec4(tc[0].color[0],tc[0].color[1],tc[0].color[2],tc[0].color[3]),0.125f);
    ImPlot::PlotLine("##scopeplot", oscData[0] + (oscDataSize - tc[0].traceSize), oscData[1] + (oscDataSize - tc[0].traceSize), tc[0].traceSize,ImPlotFlags_NoLegend);
    ImPlot::EndPlot();
  }
  ImGui::End();
}