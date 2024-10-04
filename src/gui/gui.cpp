#include "gui.h"
#include <imgui.h>

ImVec2 operator+(ImVec2 lhs, ImVec2 rhs) {
  return ImVec2(lhs.x+rhs.x,lhs.y+rhs.y);
}

ImVec2 operator+(ImVec2 lhs, float rhs) {
  return ImVec2(lhs.x+rhs,lhs.y+rhs);
}

bool USCGUI::isRunning() {
  return running;
}

USCGUI::USCGUI(unscopeParams params) {
  renderer = (USCRenderers)params.renderer;
  isGood = false;
  err = 0;
  channels = params.channels;

  sc.plotFlags = ImPlotFlags_NoLegend|ImPlotFlags_NoMenus;
  sc.scopeFlags = ImPlotAxisFlags_AutoFit|ImPlotAxisFlags_Lock|ImPlotAxisFlags_NoMenus|ImPlotAxisFlags_Foreground;

  sampleRate = params.sampleRate;

  tc = new traceParams[channels];

  for (unsigned char i = 0; i < channels; i++) {
    tc[i].enable = true;
    tc[i].yScale = params.scale;
    tc[i].yOffset = 0;
    tc[i].timebase = params.timebase;
    tc[i].trigger = 0;
    tc[i].traceSize = sampleRate*tc[i].timebase/1000;
    tc[i].trigHoldoff = 0;
    tc[i].traceOffset = tc[i].traceSize/2;
    tc[i].trigOffset = 0;
    tc[i].triggerEdge = true;
  }

  tc[0].color = ImVec4(0.13f,0.97f,0.21f,0.95f);
  if (channels > 1) {
  tc[1].color = ImVec4(0.93f,0.17f,0.23f,0.95f);
  }

  xyp.color = ImVec4(0.13f,0.97f,0.21f,0.35f);
  xyp.xOffset = 0;
  xyp.yOffset = 0;
  xyp.xScale = 1.0f;
  xyp.yScale = 1.0f;
  xyp.persistence = params.xyPersist;
  xyp.sampleLen = sampleRate*xyp.persistence/1000;
  xyp.xChan = 1;
  xyp.yChan = 2;

  wo.auxScopeOpen = false;
  wo.mainScopeOpen = true;
  wo.chanControlsOpen[0] = true;
  wo.chanControlsOpen[1] = true;
  wo.chanControlsOpen[2] = true;
  wo.xyScopeOpen = true;
  wo.xyScopeControlsOpen = true;
  wo.globalControlsOpen = true;

  oscDataSize = params.audioBufferSize;

  oscData = new float*[channels];
  oscAuxData = new float*[channels];
  oscAlign = new float*[oscDataSize];
  if (!oscData || !oscAuxData || !oscAlign) {
    return;
  }
  for (unsigned char i = 0; i < channels; i++) {
    oscData[i] = new float[oscDataSize];
    oscAuxData[i] = new float[oscDataSize];
    oscAlign[i] = new float[oscDataSize];

    if (!oscData[i] || !oscAuxData[i] || !oscAlign[i]) {
      return;
    }

    memset(oscData[i],0,oscDataSize*sizeof(float));
    memset(oscAuxData[i],0,oscDataSize*sizeof(float));
    memset(oscAlign[i],0,oscDataSize*sizeof(float));
  }
  running = false;
  updateAudio = true;
  restartAudio = true;
  audioLoopback = false;

  device    = 0;
  deviceNum = 0;
  devs.clear();
  showTrigger  = false;
  shareParams  = true;
  shareTrigger = 1;
  triggerMode  = TRIGGER_AUTO;

  fullscreen = false;

  trigColor = ImVec4(0,0,0,0);

  ai = NULL;
}

void USCGUI::attachAudioInput(USCAudioInput *i) {
  ai = i;
}

void USCGUI::setupRenderer(USCRenderers r) {
  switch (r) {
#ifdef USE_OPENGL
    case USC_REND_OPENGL_SDL:
      rd = new USCRendOGL_SDL;
      break;
#endif
#ifdef USE_DIRECTX
    case USC_REND_DIRECTX11_SDL:
        rd = new USCRendDirectX;
        break;
#endif
    default: break;
  }
}


int USCGUI::init() {
  setupRenderer(renderer);
  isGood = true;
  if (!isGood) return -1;
  if (running) return 0;
  if (rd->setup()!=0) return UGUIERROR_SETUPFAIL;
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
  if (rd->init()!=0) return UGUIERROR_INITFAIL;
  bgColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  running = true;
  io = ImGui::GetIO();
  return 0;
}

void USCGUI::getDevices(std::vector<DeviceEntry> d) {
  devs = d;
}

void USCGUI::doFrame() {
  running = rd->beginFrame();
  if (!running) return;
  ImGui::NewFrame();
  
  ImGui::DockSpaceOverViewport(ImGui::GetWindowViewport(),0);

  USCGUI::drawGUI();

  ImGui::Render();
  rd->endFrame(io,bgColor);
}

void USCGUI::drawGUI() {
  if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
    fullscreen = !fullscreen;
    rd->doFullscreen(fullscreen);
  }
  // if (devs[device].shouldPassThru) {
  //   ImGui::SameLine();
  //   ImGui::Checkbox("##lp",&audioLoopback);
  //   if (ImGui::IsItemHovered()) {
  //     ImGui::SetTooltip("enable audio loopback");
  //   }
  // } else {
  //   audioLoopback = false;
  // }
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Scopes")) {
      ImGui::MenuItem("Main Scope",NULL,&wo.mainScopeOpen);
      ImGui::MenuItem("Scope (XY)",NULL,&wo.xyScopeOpen);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Controls")) {
      for (unsigned char i = 0; i < channels; i++) {
        char buf[32];
        sprintf(buf,"Channel %d Controls",i+1);
        ImGui::MenuItem(buf,NULL,&wo.chanControlsOpen[i]);
      }
      ImGui::MenuItem("XY Scope Controls",NULL,&wo.xyScopeControlsOpen);
      ImGui::MenuItem("Global Controls",NULL,&wo.globalControlsOpen);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  drawGlobalControls();
  drawChanControls();
  drawXYScopeControls();

  for (unsigned char j = 0; j < channels; j++) {
    ai->setUpdateState(updateAudio);
    ai->setAlignParams(j,
      AlignParams((triggerMode==TRIGGER_NONE)?-INFINITY:tc[j].trigger,
      tc[j].traceSize,
      tc[j].traceOffset,
      tc[j].triggerEdge,
      tc[j].trigHoldoff,
      triggerMode!=TRIGGER_NORMAL));
    writeOscData(j,
      ai->getAlignRamp(j),
      ai->getData(j));
  }

  ImPlot::CreateContext();
    drawMainScope();
    drawXYScope();
  ImPlot::DestroyContext();
  // ImGui::Begin("align");
  // ImGui::PlotLines("align",oscAlign[0],65536,0,NULL,-1.0f,1.0f,ImGui::GetContentRegionAvail());
  // ImGui::End();
  // ImGui::ShowMetricsWindow();
}

void USCGUI::drawTriggerLamp(unsigned char chan) {
  ImDrawList* dl = ImGui::GetWindowDrawList();
  dl->AddCircleFilled(ImGui::GetCursorScreenPos()+10.0f, 10.0f, 
    ai->didTrigger(chan)?0xff00ffff:0xee112224);
}

void USCGUI::writeOscData(unsigned char chan, float* datax, float* datay) {
  memcpy(oscAlign[chan],datax,oscDataSize*sizeof(float));
  memcpy(oscData[chan],datay,oscDataSize*sizeof(float));
}

void USCGUI::writeOscAuxData(unsigned char chan, float* data) {
  memcpy(oscData[chan],data,oscDataSize*sizeof(float));
}

bool USCGUI::doRestartAudio() {
  return restartAudio;
}

int USCGUI::getAudioDeviceSetting() {
  return device;
}

void USCGUI::setAudioDeviceSetting(int d) {
  device = d;
  for (int i = 0; i < devs.size(); i++) {
    if (devs[i].dev == device) {
      deviceNum = i;
      break;
    }
  }
}

USCGUI::~USCGUI() {
  // ImGui::SaveIniSettingsToDisk(INIFILE);
  // Cleanup
  if (isGood) rd->deinit();
  delete rd;

  if (oscData) {
    for (unsigned char i = 0; i < channels; i++) {
      if (oscData[i]) {
        delete[] oscData[i];
        oscData[i] = NULL;
      }
    }
    delete[] oscData;
    oscData = NULL;
  }
  if (oscAuxData) {
    for (unsigned char i = 0; i < channels; i++) {
      if (oscAuxData[i]) {
        delete[] oscAuxData[i];
        oscAuxData[i] = NULL;
      }
    }
    delete[] oscAuxData;
    oscAuxData = NULL;
  }
  if (oscAlign) {
    for (unsigned char i = 0; i < channels; i++) {
      if (oscAlign[i]) {
        delete[] oscAlign[i];
        oscAlign[i] = NULL;
      }
    }
    delete[] oscAlign;
    oscAlign = NULL;
  }
  ai = NULL;

  if (tc) {
    delete[] tc;
    tc = NULL;
  }
}