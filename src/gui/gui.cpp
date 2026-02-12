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

#include "gui.h"
#include "imgui.h"

bool USCGUI::isRunning() {
  return running;
}

USCGUI::USCGUI() {
  isGood = false;
  // generateFFTFrequencies();

  // wo.chanControlsOpen=new bool[channels];
  // if (!wo.chanControlsOpen) return;
  // FOR_RANGE(channels) wo.chanControlsOpen[z]=true;
  wo.xyScopeOpen         = true;
  wo.xyScopeControlsOpen = true;
  wo.globalControlsOpen  = true;
  wo.aboutOpen           = false;
  wo.cursorsOpen         = true;
  wo.audioConfigOpen     = false;
  wo.spectrumOpen        = true;
  wo.spectrumControlsOpen= true;
#ifdef PROGRAM_DEBUG
  wo.metricsOpen         = false;
  wo.paramDebugOpen      = false;
  wo.triggerDebugOpen    = false;
  wo.fftDebugOpen        = false;
#endif
  running = false;

  shareParams  = true;
  shareTrigger = 1;

  doFallback = true;
  singleShot = false;


  fullscreen = false;

  settings.msDiv=false;

  errorText="";

  topKeyColor=ImVec4(0,0,0,.1);
  bottomKeyColor=ImVec4(1,1,1,.1);

  pianoColors[0]=&bottomKeyColor;
  pianoColors[1]=&topKeyColor;
  pianoColors[2]=&bottomKeyColor;
  pianoColors[3]=&topKeyColor;
  pianoColors[4]=&bottomKeyColor;
  pianoColors[5]=&bottomKeyColor;
  pianoColors[6]=&topKeyColor;
  pianoColors[7]=&bottomKeyColor;
  pianoColors[8]=&topKeyColor;
  pianoColors[9]=&bottomKeyColor;
  pianoColors[10]=&topKeyColor;
  pianoColors[11]=&bottomKeyColor;


  data = NULL;
  isGood = true;
  rd = NULL;

  newScopeBuffer = NULL;
  newDriver = DATA_PORTAUDIO;
  newScopeWin = 0;

  scopeWindows = {
    ScopeWindow(0)
  };

#ifdef PROGRAM_DEBUG
  debuggedChannel=0;
#endif
}

void USCGUI::attachData(USCData *i) {
  data = i;
}

void USCGUI::setupRenderer(USCRenderers r) {
  switch (r) {
#ifdef USE_OPENGL
    case USC_RENDER_OPENGL2:
      rd = new USCRenderOpenGL2;
      break;
#endif
#ifdef USE_DIRECTX9
    case USC_RENDER_DIRECTX9:
        rd = new USCRenderDirectX9;
        break;
#endif
#ifdef USE_DIRECTX11
    case USC_RENDER_DIRECTX11:
        rd = new USCRenderDirectX11;
        break;
#endif
    case USC_RENDER_SDLRENDERER2:
      rd = new USCRenderSDLRenderer;
      break;
    case USC_RENDER_NONE:
    default: break;
  }
}

int USCGUI::init(USCRenderers rend) {
  if (running) return 0;
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS)!=0) return -1;
  renderer = rend;
  setupRenderer(renderer);
  if (!isGood) return -1;
  if (rd->initRender()!=0) return -1;
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
  ImGui::GetIO().IniFilename = NULL;
  // ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
  ImGui::StyleColorsDark();
  style = ImGui::GetStyle();
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
  ImGui::LoadIniSettingsFromMemory(windowLayout);
  // ImGui::LoadIniSettingsFromDisk(INIFILE);
  if (rd->setupRender(
    (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI),
    PROGRAM_NAME_AND_VER,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    PROGRAM_WIDTH, PROGRAM_HEIGHT)!=0) return -1;
  bgColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  running = true;

  dummyBuffer.init(65536, 48000, "Dummy Buffer");
  float v=0.0f;
  for (int i=0; i<65536; i++) {
    v=sinf(i*M_2_PI/64.0f);
    dummyBuffer.write(&v);
  }
  return 0;
}

void USCGUI::doFrame() {
  int s;
  s=rd->renderPreLoop();
  switch (s) {
    case 1:
      if (!(ImGui::GetIO().ConfigFlags&ImGuiConfigFlags_ViewportsEnable)) SDL_Delay(100); // replace with viewports setting later
      break;
    case 0: break;
    default:
      running=false;
      break;
  }
  if (!running) return;
  ImGui::NewFrame();
  
  ImGui::DockSpaceOverViewport(ImGui::GetID("dock"),ImGui::GetMainViewport());

  drawGUI();

  ImGui::Render();
  running &= rd->renderPostLoop()>=0;
}

void USCGUI::doFullscreen() {
  fullscreen = !fullscreen;
  SDL_SetWindowFullscreen(rd->getWindow(),fullscreen?(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP):0);
}

void USCGUI::drawGUI() {
  if (ImGui::IsKeyPressed(ImGuiKey_F11)) doFullscreen();
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("Audio Configuration", NULL, &wo.audioConfigOpen);
      ImGui::Separator();
      if (ImGui::MenuItem("Fullscreen", "F11", fullscreen)) doFullscreen();
      if (ImGui::MenuItem("Quit")) running = false;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Scopes")) {
      char strbuf[64];
      const int scopeWins = scopeWindows.size();
      for (int i=0; i<scopeWins; i++) {
        snprintf(strbuf, 64, "Scope Window %d", i+1);
        ImGui::MenuItem(strbuf, NULL, &scopeWindows[i].windowOpen);
      }
      if (ImGui::MenuItem("New Scope Window")) {
        scopeWindows.push_back(ScopeWindow(scopeWins));
      }
      ImGui::Separator();
      ImGui::MenuItem("Scope (XY)",NULL,&wo.xyScopeOpen);
      ImGui::Separator();
      ImGui::MenuItem("Frequency Spectrum",NULL,&wo.spectrumOpen);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Controls")) {
      char buf[64];
      ImGui::MenuItem("XY Scope Controls");
      ImGui::Separator();
      ImGui::MenuItem("Spectrum Controls",NULL,&wo.spectrumControlsOpen);
      ImGui::Separator();
      ImGui::MenuItem("Global Controls",NULL,&wo.globalControlsOpen);
      ImGui::MenuItem("Cursors",NULL,&wo.cursorsOpen);
      ImGui::EndMenu();
    }
#ifdef PROGRAM_DEBUG
    if (ImGui::BeginMenu("Debug")) {
      ImGui::MenuItem("Metrics",NULL,&wo.metricsOpen);
      ImGui::Separator();
      ImGui::MenuItem("Parameters...",NULL,&wo.paramDebugOpen);
      ImGui::MenuItem("Trigger...",NULL,&wo.triggerDebugOpen);
      ImGui::MenuItem("FFT...",NULL,&wo.fftDebugOpen);
      ImGui::EndMenu();
    }
#endif
    if (ImGui::BeginMenu("About")) {
      ImGui::MenuItem("About...",NULL,&wo.aboutOpen);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  drawChannelManager(&wo.globalControlsOpen);
  drawXYScopeControls(&wo.xyScopeControlsOpen);
  // drawSpectrumControls(&wo.spectrumControlsOpen);

  drawAbout(&wo.aboutOpen);
  drawAudioConfig(&wo.audioConfigOpen);
  drawChanControls();

  for (ScopeWindow& s:scopeWindows) s.drawScope(true);
  drawXYScope(&wo.xyScopeOpen);
  // drawSpectrum(&wo.spectrumOpen);
#ifdef PROGRAM_DEBUG
  if (wo.metricsOpen) ImGui::ShowMetricsWindow(&wo.metricsOpen);
  drawTriggerDebug(&wo.triggerDebugOpen);
  // drawParamDebug(&wo.paramDebugOpen);
  // drawFFTDebug(&wo.fftDebugOpen);
#endif
  if (ImGui::BeginPopupModal("Error##ERRPOPUP", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize)) {
    ImGui::Text("%s", errorText.c_str());
    if (ImGui::Button("OK")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  // drawScopeDebugImFrustrated();
}

// void USCGUI::drawScopeDebugImFrustrated() {
//   if (data==NULL) return;
//   if (data->getDriverCount()==0) return;
//   if (ImGui::Begin("pain")) {
//     char sbuf[256];
//     for (int i=0; i<data->getDriver(0)->getBufferCount(); i++) {      
//       DataBuffer* buf = data->getDriver(0)->getBuffer(i);
//       if (!buf) {
//         ImGui::Text("buffer %d NULL!", i);
//         continue;
//       }
//       ImGui::Text("buffer %d (%p): %p\n%llu samples", i, buf,buf->getBuffer(),buf->getSize());  
//       if (buf->getBuffer()==NULL) continue;
//       snprintf(sbuf, 256, "CH %d", i);
//       ImGui::PlotLines(sbuf, (float*)buf->getBuffer(), buf->getSize());
//     }
//   }
//   ImGui::End();
// }

#ifdef PROGRAM_DEBUG
void USCGUI::drawTriggerDebug(bool* open) {
  // very slow indeed
  if (!*open) return;
#define DIV 4
  if (ImGui::Begin("Trigger Debug", open)) {
    if (ImGui::InputInt("channel", &debuggedChannel)) {
      if (debuggedChannel<0) debuggedChannel=0;
      if (debuggedChannel>scopes.size()-1) debuggedChannel=scopes.size()-1;
    }

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImRect rect(origin, origin+ImGui::GetContentRegionAvail());
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (!scopes.empty()) {
      ScopeChannel* chan = scopes[debuggedChannel];
      const float* bufferData = (float*)chan->getBuffer()->getBuffer();
      const nint bufferSize = chan->getBuffer()->getSize()/DIV;
      ImVec2* plot = new ImVec2[bufferSize];

      for (nint i=0; i<bufferSize; i++) {
        plot[i] = ImVec2(
          ImLerp(rect.Min.x, rect.Max.x, (float)i/bufferSize),
          ImLerp(rect.GetCenter().y, rect.Min.y, bufferData[i*DIV])
        );
      }

      dl->AddPolyline(plot, bufferSize, 0xffffffff, 0, 1.0f);

      Trigger* trig = chan->getTrigger();
      ImGui::Text("trigger: %p", trig);
      if (trig) {
        nint needlePos = (chan->getNeedle()+chan->getBuffer()->getIndex())%chan->getBuffer()->getSize();
        ImGui::Text("needle: %llu (%llu)", needlePos, trig->getTriggerIndex());
        float trigNeedle = (float)needlePos/chan->getBuffer()->getSize();
        trigNeedle = ImLerp(rect.Min.x, rect.Max.x, trigNeedle);
        dl->AddLine(
          ImVec2(trigNeedle,rect.Min.y),
          ImVec2(trigNeedle,rect.Max.y),
          0xff00ff00, 2.0f
        );
      }

      delete[] plot;
    }
  }
  ImGui::End();
#undef DIV
}

// void USCGUI::drawParamDebug(bool* open) {
//   if (!*open) return;
//   ImGui::Begin("params",open);
//   if (ImGui::BeginTable("##paramTable", 6)) {
//     ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
//     ImGui::TableNextColumn();
//     ImGui::Text("ch");
//     ImGui::TableNextColumn();
//     ImGui::Text("label");
//     ImGui::TableNextColumn();
//     ImGui::Text("type");
//     ImGui::TableNextColumn();
//     ImGui::Text("value");
//     ImGui::TableNextColumn();
//     ImGui::Text("hovered");
//     ImGui::TableNextColumn();
//     ImGui::Text("active");
//     FOR_RANGE(channels) {
//       ImGui::PushID(z);
//       for (TriggerParam p:trigger[z]->getParams()) {
//         ImGui::TableNextRow();
//         ImGui::TableNextColumn();
//         ImGui::Text("%d",z);
//         ImGui::TableNextColumn();
//         ImGui::Text("%s",p.getLabel());
//         ImGui::TableNextColumn();
//         ImGui::Text("%d",p.getType());
//         ImGui::TableNextColumn();
//         ImGui::PushID(p.getLabel());
//         switch (p.getType()) {
//           case PARAM_KNOBNORM:
//             if (ImGui::InputFloat("##floatinput", (float*)p.getValuePtr())) {
//               if (p.getValue<float>()<-1.0f) p.setValue<float>(-1.0f);
//               if (p.getValue<float>()>1.0f) p.setValue<float>(1.0f);
//             }
//             break;
//           case PARAM_KNOBUNIT:
//             if (ImGui::InputFloat("##floatinput", (float*)p.getValuePtr())) {
//               if (p.getValue<float>()<0.0f) p.setValue<float>(0.0f);
//               if (p.getValue<float>()>1.0f) p.setValue<float>(1.0f);
//             }
//             break;
//           case PARAM_TOGGLE:
//             ImGui::Checkbox("##boolinput", (bool*)p.getValuePtr());
//             break;
//           default:
//             ImGui::Text("%p",p.getValuePtr());
//             break;
//         }
//         ImGui::PopID();
//         ImGui::TableNextColumn();
//         ImGui::Text(p.isHovered()?"true":"false");
//         ImGui::TableNextColumn();
//         ImGui::Text(p.isActive()?"true":"false");
//       }
//       ImGui::PopID();
//     }
//     ImGui::EndTable();
//   }
//   ImGui::End();
// }
#endif

void USCGUI::errorPopup(const char* errorTxt, ...) {
  va_list args;
  va_start(args, errorTxt);
  size_t len = vsnprintf(NULL, 0, errorTxt, args);
  va_end(args);
  errorText.reserve(len+1);
  errorText.resize(len);
  vsnprintf(&errorText[0], len, errorTxt, args);
  ImGui::OpenPopup("Error##ERRPOPUP");
}

USCGUI::~USCGUI() {
  if (isGood) {
    if (rd) rd->destroyRender();
    DELETE_PTR(rd)
  }
  for (ScopeChannel*& chan:scopes) {
    if (chan) delete chan;
  }
}
