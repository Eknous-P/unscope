#include "gui.h"

ImVec2 operator+(ImVec2 lhs, ImVec2 rhs) {
  return ImVec2(lhs.x+rhs.x,lhs.y+rhs.y);
}

ImVec2 operator+(ImVec2 lhs, float rhs) {
  return ImVec2(lhs.x+rhs,lhs.y+rhs);
}

bool USCGUI::isRunning() {
  return running;
}

USCGUI::USCGUI(unscopeParams *params) {
  up = params;
  renderer = (USCRenderers)up->renderer;
  isGood = false;
  err = 0;
  channels = up->channels;

  sc.plotFlags = ImPlotFlags_NoLegend|ImPlotFlags_NoMenus;
  sc.scopeFlags = ImPlotAxisFlags_AutoFit|ImPlotAxisFlags_Lock|ImPlotAxisFlags_NoMenus|ImPlotAxisFlags_Foreground;

  sampleRate = up->sampleRate;

  tc = new traceParams[channels];

  for (unsigned char i = 0; i < channels; i++) {
    tc[i].enable = true;
    tc[i].yScale = up->scale;
    tc[i].yOffset = 0;
    tc[i].timebase = up->timebase;
    tc[i].traceSize = sampleRate * tc[i].timebase / 1000.0f;
  }

  xyp.xOffset = 0;
  xyp.yOffset = 0;
  xyp.xScale = 1.0f;
  xyp.yScale = 1.0f;
  xyp.persistence = up->xyPersist;
  xyp.sampleLen = sampleRate * xyp.persistence / 1000.0f;
  xyp.axisChan[0] = 1;
  xyp.axisChan[1] = 2;

  oscDataSize = up->audioBufferSize;

  bufferTime = (float)oscDataSize / (float)sampleRate * 1000.0f;

  oscData = new float*[channels];
  oscAlign = new float*[channels];
  if (!oscData || !oscAlign) {
    return;
  }
  FOR_RANGE(channels) {
    oscData[z] = new float[oscDataSize];
    if (oscData[z] == NULL) return;
    memset(oscData[z],0,oscDataSize*sizeof(float));
  }
  running = false;
  updateAudio = true;
  restartAudio = true;
  audioLoopback = false;

  device    = 0;
  deviceNum = 0;
  devs.clear();
  devs_c = NULL;
  showTrigger  = false;
  shareParams  = true;
  shareTrigger = 1;
  trigNum      = TRIG_ANALOG;
  triggerSet   = false;

  doFallback = true;
  singleShot = false;

  fallbackTrigger = new Trigger*[channels];
  trigger = new Trigger*[channels];

  if (!fallbackTrigger) return;
  FOR_RANGE(channels) {
    Trigger* t = new TriggerFallback;
    if (!t) return;
    t->setupTrigger(oscDataSize, oscData[z]);

    fallbackTrigger[z] = t;
  }


  ai = NULL;
  cf = NULL;
  isGood = true;
}

void USCGUI::attachAudioInput(USCAudioInput *i) {
  ai = i;
}

void USCGUI::attachConfig(USCConfig *c) {
  cf = c;
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

void USCGUI::setupTrigger(Triggers t) {
  // destroy current trigger
  if (triggerSet && trigger) {
    for (unsigned char z = 0; z < channels; z++) {
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
      default:
        tp = new TriggerFallback;
        break;
    }
    if (!tp) return;
    tp->setupTrigger(oscDataSize, oscData[z]);

    trigger[z] = tp;
  }
  triggerSet = true;
}


int USCGUI::init() {
  setupRenderer(renderer);
  setupTrigger(trigNum);
  if (!isGood) return -1;
  if (running) return 0;
  if (rd->setup()!=0) return UGUIERROR_SETUPFAIL;
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard|ImGuiConfigFlags_DockingEnable;
  ImGui::GetIO().IniFilename = NULL;
  if (up->enableMultiViewport) {
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    io.ConfigViewportsNoAutoMerge = true;
    io.ConfigViewportsNoTaskBarIcon = true;
  }
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
  // Setup Platform/Renderer backends
  if (rd->init()!=0) return UGUIERROR_INITFAIL;
  bgColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  running = true;
  io = ImGui::GetIO();
  rd->doFullscreen(up->fullscreen);

  if (cf) {
    std::string layout = cf->getLayout();
    if (layout.size()>0) {
      printf("%s\nsize: %lu\n", layout.c_str(), layout.size());
      // ImGui::LoadIniSettingsFromMemory(layout.c_str());
    }
  }
  return 0;
}

void USCGUI::getDevices(std::vector<DeviceEntry> d) {
  devs = d;
  devs_c = new char*[devs.size()];
  FOR_RANGE(devs.size()) {
    devs_c[z] = new char[2048];
    strncpy(devs_c[z], devs[z].devName.c_str(), 2048);
  }
  cf->settings[0].settings.push_back(
    Setting(SETTING_SELECTABLE_STRING, "device", "device", NULL, &deviceNum, devs_c, devs.size())
  );
}

void USCGUI::doFrame() {
  running = rd->beginFrame();
  if (!running) return;
  ImGui::NewFrame();
  
  ImGui::DockSpaceOverViewport();

  USCGUI::drawGUI();

  ImGui::Render();
  rd->endFrame(io,bgColor);
}

void USCGUI::drawGUI() {
  if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
    up->fullscreen = !up->fullscreen;
    rd->doFullscreen(up->fullscreen);
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
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("Settings...", NULL, &up->settingsOpen);
      if (ImGui::MenuItem("Exit")) running = false;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Scopes")) {
      ImGui::MenuItem("Main Scope",NULL,&up->mainScopeOpen);
      if (channels>1) ImGui::MenuItem("Scope (XY)",NULL,&up->xyScopeOpen);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Controls")) {
      for (unsigned char i = 0; i < channels; i++) {
        char buf[32];
        sprintf(buf,"Channel %d Controls",i+1);
        ImGui::MenuItem(buf,NULL,&up->chanControlsOpen[i]);
      }
      ImGui::MenuItem("XY Scope Controls",NULL,&up->xyScopeControlsOpen);
      ImGui::MenuItem("Global Controls",NULL,&up->globalControlsOpen);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("About")) {
      ImGui::MenuItem("About...",NULL,&up->aboutOpen);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  drawGlobalControls();
  drawChanControls();
  drawXYScopeControls();

  drawAbout();
  drawSettings();

  ai->setUpdateState(updateAudio);
  setOscData(ai->getData());

  FOR_RANGE(channels) {
    Trigger* t = trigger[z];
    if (!t->trigger(tc[z].traceSize)) {
      if (doFallback) {
        t = fallbackTrigger[z];
        t->trigger(tc[z].traceSize);
      }
    }
    oscAlign[z] = t->getAlignBuffer();
  }

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
  
  // drawAlignDebug();
  
  ImGui::ShowMetricsWindow();
}

void USCGUI::drawAlignDebug() {
  // very slow indeed
  if (!(oscAlign && oscData)) return;
#define DIV 4
  ImGui::Begin("align", NULL, ImGuiWindowFlags_NoTitleBar);
  ImDrawList* dl = ImGui::GetWindowDrawList();
  unsigned long int count = oscDataSize / DIV;
  ImVec2 *al = new ImVec2[count], *ol = new ImVec2[count];
  ImVec2 winSize = ImGui::GetWindowSize(), winPos = ImGui::GetWindowPos();
  if (!(al && ol)) return;
  for (unsigned long int i = 0; i < count; i++) {
    al[i] = ImVec2(((float)i/count)*winSize.x,winSize.y*clamp((1.0f-oscAlign[0][i*DIV])/2.f))+winPos;
    ol[i] = ImVec2(((float)i/count)*winSize.x,winSize.y*(1.0f-oscData[0][i*DIV])/2.0f)+winPos;
  }

  dl->AddPolyline(ol, count, 0x7f7777ff, ImDrawFlags_None, 1.0f);
  dl->AddPolyline(al, count, 0xff77ffff, ImDrawFlags_None, 1.0f);

  delete[] al;
  delete[] ol;
  ImGui::End();
#undef DIV
}

void USCGUI::setOscData(float** d) {
  if (d == NULL) return;
  FOR_RANGE(channels) memcpy(oscData[z], d[z], oscDataSize*sizeof(float));
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

void USCGUI::deinint() {
  const char* layout = ImGui::SaveIniSettingsToMemory();
  cf->saveLayout(layout);
}

USCGUI::~USCGUI() {
  // Cleanup
  if (isGood) rd->deinit();
  if (rd) {
    delete rd;
    rd = NULL;
  }
  if (trigger) {
    for (unsigned char z = 0; z < channels; z++) {
      if (trigger[z]) {
        delete trigger[z];
        trigger[z] = NULL;
      }
    }
    delete[] trigger;
    trigger = NULL;
  }
  if (fallbackTrigger) {
    for (unsigned char z = 0; z < channels; z++) {
      if (fallbackTrigger[z]) {
        delete fallbackTrigger[z];
        fallbackTrigger[z] = NULL;
      }
    }
    delete[] fallbackTrigger;
    fallbackTrigger = NULL;
  }

  DELETE_DOUBLE_PTR(oscData, channels)
  DELETE_PTR(oscAlign)
  ai = NULL;
  cf = NULL;

  DELETE_PTR(tc)
  DELETE_DOUBLE_PTR(devs_c, devs.size())
}