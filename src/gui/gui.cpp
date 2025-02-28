#include "gui.h"

bool USCGUI::isRunning() {
  return running;
}

USCGUI::USCGUI(unscopeParams *params) {
  up = params;
  renderer = (USCRenderers)up->renderer;
  isGood = false;
  err = 0;
  channels = up->channels;

  sc.plotFlags = ImPlotFlags_NoLegend|ImPlotFlags_NoMenus|ImPlotFlags_NoMouseText;
  sc.scopeFlags = ImPlotAxisFlags_AutoFit|ImPlotAxisFlags_Lock|ImPlotAxisFlags_NoMenus|ImPlotAxisFlags_Foreground;

  sampleRate = up->sampleRate;

  tc = new traceParams[channels];

  for (unsigned char i = 0; i < channels; i++) {
    tc[i].enable = true;
    tc[i].yScale = up->scale;
    tc[i].yOffset = 0;
    tc[i].timebase = up->timebase;
    tc[i].traceSize = sampleRate*tc[i].timebase/1000;
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
  xyp.persistence = up->xyPersist;
  xyp.sampleLen = sampleRate*xyp.persistence/1000;
  xyp.axisChan[0] = 1;
  xyp.axisChan[1] = 2;

  wo.mainScopeOpen = true;
  wo.chanControlsOpen[0] = true;
  wo.chanControlsOpen[1] = true;
  wo.chanControlsOpen[2] = true;
  wo.xyScopeOpen = true;
  wo.xyScopeControlsOpen = true;
  wo.globalControlsOpen = true;
  wo.aboutOpen=false;
  wo.cursorsOpen=true;

  oscDataSize = up->audioBufferSize;

  oscData = new float*[channels];
  oscAlign = new float*[channels];
  if (!oscData || !oscAlign) {
    return;
  }
  FOR_RANGE(channels) {
    oscData[z] = new float[oscDataSize];
    // oscAlign[i] = new float[oscDataSize];

    if (oscData[z] == NULL/* || !oscAlign[i]*/) {
      return;
    }

    memset(oscData[z],0,oscDataSize*sizeof(float));
    // memset(oscAlign[i],0,oscDataSize*sizeof(float));
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
  trigNum      = TRIG_ANALOG;
  triggerSet   = false;

  doFallback = true;
  singleShot = false;

  fullscreen = false;

  fallbackTrigger = new Trigger*[channels];
  trigger = new Trigger*[channels];

  if (!fallbackTrigger) return;
  FOR_RANGE(channels) {
    Trigger* t = new TriggerFallback;
    if (!t) return;
    t->setupTrigger(up, oscData[z]);

    fallbackTrigger[z] = t;
  }

  HCursors[0]=plotCursor("X1",-.5f);
  HCursors[1]=plotCursor("X2",.5f);
  VCursors[0]=plotCursor("Y1",-.5f);
  VCursors[1]=plotCursor("Y2",.5f);

  showHCursors = false;
  showVCursors = false;

  ai = NULL;
  isGood = true;
}

void USCGUI::attachAudioInput(USCAudioInput *i) {
  ai = i;
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
    tp->setupTrigger(up, oscData[z]);

    trigger[z] = tp;
  }
  triggerSet = true;
}


int USCGUI::init() {
  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0) return UGUIERROR_INITFAIL;
  setupRenderer(renderer);
  setupTrigger(trigNum);
  if (!isGood) return -1;
  if (running) return 0;
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
    PROGRAM_NAME,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    PROGRAM_WIDTH, PROGRAM_HEIGHT)!=0) return UGUIERROR_SETUPFAIL;
  bgColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  running = true;
  io = ImGui::GetIO();
  return 0;
}

void USCGUI::getDevices(std::vector<DeviceEntry> d) {
  devs = d;
}

void USCGUI::doFrame() {
  running = rd->renderPreLoop()>=0;
  if (!running) return;
  ImGui::NewFrame();
  
  ImGui::DockSpaceOverViewport(ImGui::GetWindowViewport(),0);

  USCGUI::drawGUI();

  ImGui::Render();
  running = rd->renderPostLoop()>=0;
}

void USCGUI::drawGUI() {
  if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
    fullscreen = !fullscreen;
    SDL_SetWindowFullscreen(rd->getWindow(),fullscreen?(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP):0);
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
  
  // ImGui::ShowMetricsWindow();
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

USCGUI::~USCGUI() {
  if (isGood) rd->destroyRender();
  DELETE_PTR(rd)
  DELETE_DOUBLE_PTR(trigger, channels)
  DELETE_DOUBLE_PTR(fallbackTrigger, channels)
  DELETE_DOUBLE_PTR_ARR(oscData, channels)
  DELETE_PTR_ARR(oscAlign)
  ai = NULL;

  DELETE_PTR_ARR(tc)
}