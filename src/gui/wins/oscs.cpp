#include "gui.h"

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
      if (oscAlign[i] && oscData[i] && oscDataSize) {
        ImPlot::PlotLine("##scopeplot", oscAlign[trigChan], oscData[i], oscDataSize,ImPlotFlags_NoLegend);
      }
      ImVec4 trigColor = trigger[i]->getTriggered()?ImVec4(0,1,0,.5f):ImVec4(1,0,0,.5f);
      for (TriggerParam p : trigger[i]->getParams()) {
        if (shareTrigger > 0 && i != shareTrigger - 1) continue;
        double valD = *(float*)p.getValue();
        if (p.bindToDragX) {
          if (ImPlot::DragLineX(2*i+1, &valD, tc[i].color)) *(float*)p.getValue() = valD;
          // ImPlot::TagX(valD, trigColor, "CH %d", i + 1);
        }
        if (p.bindToDragY) {
          if (ImPlot::DragLineY(2*i+1, &valD, trigColor)) *(float*)p.getValue() = valD;
          ImPlot::TagY(valD, trigColor, "CH %d", i + 1);
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
