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

bool USCGUI::isRunning() {
  return running;
}

USCGUI::USCGUI(unscopeParams *params, AudioConfig *aConf) {
  up = params;
  audConf = aConf;
  renderer = (USCRenderers)up->renderer;
  isGood = false;
  channels = aConf->inputChannels;

  sc.plotFlags = ImPlotFlags_NoLegend|ImPlotFlags_NoMenus|ImPlotFlags_NoMouseText;
  sc.scopeFlags = ImPlotAxisFlags_AutoFit|ImPlotAxisFlags_Lock|ImPlotAxisFlags_NoMenus|ImPlotAxisFlags_Foreground;

  sampleRate = aConf->sampleRate;

  tc = new traceParams[channels];

  for (unsigned char i = 0; i < channels; i++) {
    tc[i].enable = true;
    tc[i].yScale = up->scale;
    tc[i].yOffset = 0.0f;
    tc[i].timebase = up->timebase;
    tc[i].traceSize = sampleRate*tc[i].timebase/1000.0f;
  }

  tc[0].color = ImVec4(0.13f,0.97f,0.21f,0.95f);
  if (channels > 1) {
  tc[1].color = ImVec4(0.93f,0.17f,0.23f,0.95f);
  }

  xyp.color = ImVec4(0.13f,0.97f,0.21f,0.35f);
  xyp.xOffset = 0.0f;
  xyp.yOffset = 0.0f;
  xyp.xScale = 1.0f;
  xyp.yScale = 1.0f;
  xyp.persistence = up->xyPersist;
  xyp.sampleLen = sampleRate*xyp.persistence/1000;
  xyp.axisChan[0] = 1;
  xyp.axisChan[1] = 2;

  wo.mainScopeOpen       = true;
  wo.chanControlsOpen[0] = true;
  wo.chanControlsOpen[1] = true;
  wo.chanControlsOpen[2] = true;
  wo.xyScopeOpen         = true;
  wo.xyScopeControlsOpen = true;
  wo.globalControlsOpen  = true;
  wo.aboutOpen           = false;
  wo.cursorsOpen         = true;
  wo.audioConfigOpen     = false;

  oscDataSize = up->audioBufferSize;

  oscData = new float*[channels];
  oscAlign = new float*[channels];
  if (!oscData || !oscAlign) {
    return;
  }
  FOR_RANGE(channels) {
    oscData[z] = new float[oscDataSize];

    if (oscData[z] == NULL) {
      return;
    }

    memset(oscData[z],0,oscDataSize*sizeof(float));
  }
  running = false;
  updateAudio = true;
  restartAudio = true;
  audioLoopback = false;

  inputDeviceS  = 0;
  outputDeviceS = 0;
  showTrigger  = false;
  shareParams  = true;
  shareTrigger = 1;
  trigNum      = TRIG_ANALOG;
  triggerSet   = false;

  doFallback = true;
  singleShot = false;

  loopbackVolume = 0.0f;

  fullscreen = false;

  trigger = new Trigger*[channels];

  HCursors[0]=plotCursor("X1",-.5f);
  HCursors[1]=plotCursor("X2",.5f);
  VCursors[0]=plotCursor("Y1",-.5f);
  VCursors[1]=plotCursor("Y2",.5f);

  showHCursors = false;
  showVCursors = false;

#ifdef TRIGGER_DEBUG
  triggerDebugBegin = 60000;
  triggerDebugEnd = oscDataSize;
#endif

  memset(errorText, 0, 2048 * sizeof(char));

  ai = NULL;
  devs = NULL;
  isGood = true;
}

void USCGUI::attachAudioInput(USCAudio *i) {
  ai = i;
  devs = ai->getDevices();
}

void USCGUI::setupRenderer(USCRenderers r) {
  switch (r) {
#ifdef USE_OPENGL
    case USC_RENDER_OPENGL2:
      rd = new USCRenderOpenGL2;
      break;
#endif
#ifdef USE_DIRECTX
    case USC_RENDER_DIRECTX11:
        rd = new USCRenderDirectX11;
        break;
#endif
    case USC_RENDER_NONE:
    default: break;
  }
}

void USCGUI::setupTrigger(Triggers t) {
  // destroy current trigger
  if (triggerSet && trigger) {
    FOR_RANGE(channels) {
      if (trigger[z]) {
        delete trigger[z];
        trigger[z] = NULL;
      }
    }
  }
  // set new trigger
  triggerSet = false;
  FOR_RANGE(channels) {
    Trigger* tp;
    switch (t) {
      case TRIG_ANALOG:
        tp = new TriggerAnalog;
        break;
      case TRIG_SMOOTH:
        tp = new TriggerSmooth;
        break;
      default:
        tp = new TriggerFallback;
        break;
    }
    if (!tp) return;
    tp->setupTrigger(up, oscData[z]);

    trigger[z] = tp;
  }
  triggerSet = true;
}


int USCGUI::init() {
  if (running) return 0;
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0) return UGUIERROR_INITFAIL;
  setupRenderer(renderer);
  setupTrigger(trigNum);
  if (!isGood) return -1;
  if (rd->initRender()!=0) return UGUIERROR_INITFAIL;
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
  ImGui::GetIO().IniFilename = NULL;
  // ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
  //io.ConfigViewportsNoAutoMerge = true;
  //io.ConfigViewportsNoTaskBarIcon = true;
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();
  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
  style = ImGui::GetStyle();
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
  ImGui::LoadIniSettingsFromMemory(windowLayout);
  // ImGui::LoadIniSettingsFromDisk(INIFILE);
  // Setup Platform/Renderer backends
  if (rd->setupRender(
    (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI),
    PROGRAM_NAME_AND_VER,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    PROGRAM_WIDTH, PROGRAM_HEIGHT)!=0) return UGUIERROR_SETUPFAIL;
  bgColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  running = true;
  io = ImGui::GetIO();
  return 0;
}

void USCGUI::doFrame() {

  running &= rd->renderPreLoop()>=0;
  if (!running) return;
  ImGui::NewFrame();
  
  ImGui::DockSpaceOverViewport(ImGui::GetWindowViewport(),0);

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
      ImGui::MenuItem("Main Scope",NULL,&wo.mainScopeOpen);
      ImGui::MenuItem("Scope (XY)",NULL,&wo.xyScopeOpen);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Controls")) {
      char buf[32];
      for (unsigned char i = 0; i < channels; i++) {
        sprintf(buf,"Channel %d Controls",i+1);
        ImGui::MenuItem(buf,NULL,&wo.chanControlsOpen[i]);
      }
      ImGui::MenuItem("XY Scope Controls",NULL,&wo.xyScopeControlsOpen);
      ImGui::MenuItem("Global Controls",NULL,&wo.globalControlsOpen);
      ImGui::MenuItem("Cursors",NULL,&wo.cursorsOpen);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("About")) {
      ImGui::MenuItem("About...",NULL,&wo.aboutOpen);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  drawGlobalControls();
  drawChanControls();
  drawXYScopeControls();

  drawAbout();
  drawCursors();
  drawAudioConfig();

  if (updateAudio) setOscData(ai->getAudioBuffer());

  if (singleShot) {
    if (shareTrigger>0) {
      if (trigger[shareTrigger-1]->getTriggered()) updateAudio = false;
    } else {
      FOR_RANGE(channels) {
        if (trigger[z]->getTriggered()) {
          updateAudio = false;
          break;
        }
      }
    }
  }


  ImPlot::CreateContext();
    drawMainScope();
    drawXYScope();
  ImPlot::DestroyContext();
#ifdef TRIGGER_DEBUG
  drawTriggerDebug();
#endif
  // ImGui::ShowMetricsWindow();
}

#ifdef TRIGGER_DEBUG
void USCGUI::drawTriggerDebug() {
  // very slow indeed
  if (!(oscAlign && oscData)) return;
#define DIV 4
  ImGui::Begin("Trigger Debug");
  ImGui::InputScalar("begin", ImGuiDataType_U64,  &triggerDebugBegin);
  ImGui::InputScalar("end", ImGuiDataType_U64,  &triggerDebugEnd);
  ImDrawList* dl = ImGui::GetWindowDrawList();
  nint count = (triggerDebugEnd-triggerDebugBegin)/DIV;
  unsigned char chan = shareTrigger<0?0:shareTrigger-1;
  if (!(count < 0 || triggerDebugEnd > oscDataSize || count > oscDataSize/DIV)) {
    ImVec2 *al = new ImVec2[count],
           *ol = new ImVec2[count],
           *sl = new ImVec2[count];
    ImVec2 winSize = ImGui::GetWindowSize(), winPos = ImGui::GetWindowPos();
    float hovering = ImGui::GetIO().MousePos.x - winPos.x;
    float winTitleBar = ImGui::GetStyle().FramePadding.x*2 + ImGui::CalcTextSize("Trigger Debug").y;
    winSize.y-=winTitleBar;
    winPos.y+=winTitleBar;
    ImGui::Text("TRIGGER: %s DIV: %d, range: %llu", triggerNames[trigNum-1], DIV, count);
    if (ImGui::IsWindowHovered()) ImGui::Text("index: %llu",(nint)((hovering/winSize.x)*count*DIV) + triggerDebugBegin);
    if (al && ol && sl) {
      float x=0.0f;
      for (nint i = 0; i < count; i++) {
        // al[i] = ImVec2(x,winSize.y*clamp((1.0f-oscAlign[chan][triggerDebugBegin+i*DIV])/2.f))+winPos;
        ol[i] = ImVec2(x,winSize.y*(1.0f-oscData[chan][triggerDebugBegin+i*DIV])/2.0f)+winPos;
        if (trigNum == TRIG_SMOOTH && sl) {
          sl[i] = ImVec2(x,winSize.y*clamp((1.0f-(((TriggerSmooth**)trigger)[chan]->getSmoothBuffer())[triggerDebugBegin+i*DIV])/2.f))+winPos;
        }
        x+=winSize.x/count;
      }
      dl->AddPolyline(ol, count, 0x7f7777ff, ImDrawFlags_None, 1.0f);
      // dl->AddPolyline(al, count, 0xff77ffff, ImDrawFlags_None, 1.0f);
      switch (trigNum) {
        case TRIG_ANALOG: {
          float trigIdx = (float)(((TriggerAnalog**)trigger)[chan]->getTriggerIndex() - triggerDebugBegin)/(count*DIV);
          dl->AddLine(ImVec2(trigIdx*winSize.x,0.0f)+winPos,
                      ImVec2(trigIdx*winSize.x,winSize.y)+winPos,
                     0xffff00ff);
          break;
        }
        case TRIG_SMOOTH: {
          float trigIdx = (float)(((TriggerAnalog**)trigger)[chan]->getTriggerIndex() - triggerDebugBegin)/(count*DIV);
          dl->AddLine(ImVec2(trigIdx*winSize.x,0.0f)+winPos,
                      ImVec2(trigIdx*winSize.x,winSize.y)+winPos,
                     0xffff00ff);
          if (sl) {
            dl->AddPolyline(sl, count, 0xffff66ff, ImDrawFlags_None, 1.0f);
            dl->AddLine(ImVec2(0,winSize.y*(1.0f-((TriggerSmooth**)trigger)[chan]->getTriggerLevel())/2.f)+winPos,
                        ImVec2(winSize.x,winSize.y*(1.0f-((TriggerSmooth**)trigger)[chan]->getTriggerLevel())/2.f)+winPos, 0xffff7777);
          }
          break;
        }
        default: break;
      }

      dl->AddRectFilled(ImVec2((1.f-(float)tc[chan].traceSize/(count*DIV))*winSize.x,0.0f)+winPos,
                  ImVec2(winSize.x,winSize.y)+winPos,
                  0x1155ff22);
    } else {
      ImGui::Text("malloc fail!:\nal: %p\nol: %p\nsl: %p",al,ol,sl);
    }

    if (al) delete[] al;
    if (ol) delete[] ol;
    if (sl) delete[] sl;
    
  } else {
    ImGui::Text("invalid range!");
  }
  ImGui::End();
#undef DIV
}
#endif

void USCGUI::setOscData(float** d) {
  if (d == NULL) return;
  if (!updateAudio) return;
  FOR_RANGE(channels) memcpy(oscData[z], d[z], oscDataSize*sizeof(float));
}

bool USCGUI::doRestartAudio() {
  return restartAudio;
}

void USCGUI::errorPopup(const char* errorTxt, ...) {
  va_list args;
  va_start(args, errorTxt);
  vsnprintf(errorText, sizeof(errorText), errorTxt, args);
  va_end(args);
  ImGui::OpenPopup("Error##ERRPOPUP");
}

void USCGUI::updateAudioDevices() {
  inputDeviceS = -1;
  outputDeviceS = -1;
  for (int i = 0; i < devs->size(); i++) {
    AudioDevice d = (*devs)[i];
    if (d.dev == audConf->inputDevice && inputDeviceS == -1) {
      inputDeviceS = i;
    }
    if (d.dev == audConf->outputDevice && outputDeviceS == -1) {
      outputDeviceS = i;
    }
  }
}

USCGUI::~USCGUI() {
  if (isGood) rd->destroyRender();
  DELETE_PTR(rd)
  DELETE_DOUBLE_PTR(trigger, channels)
  DELETE_DOUBLE_PTR_ARR(oscData, channels)
  DELETE_PTR_ARR(oscAlign)
  DELETE_PTR_ARR(tc)
}
