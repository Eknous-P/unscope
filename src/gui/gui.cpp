#include "gui.h"

bool GUI::isRunning() {
  return running;
}

GUI::GUI(unsigned long int sampleRateDef, unsigned long int dataSize, unsigned char chanCount, float timebaseDef, float yScaleDef, float triggerDef) {
  err = 0;
  channels = chanCount;

  sc.plotFlags = ImPlotFlags_NoLegend|ImPlotFlags_NoMenus;
  sc.scopeFlags = ImPlotAxisFlags_AutoFit|ImPlotAxisFlags_Lock|ImPlotAxisFlags_NoMenus|ImPlotAxisFlags_Foreground;

  sampleRate = sampleRateDef;

  tc = new traceParams[channels];

  for (unsigned char i = 0; i < channels; i++) {
    tc[i].enable = true;
    tc[i].yScale = yScaleDef;
    tc[i].yOffset = 0;
    tc[i].timebase = timebaseDef;
    tc[i].trigger = 0;
    tc[i].traceSize = sampleRate*tc[i].timebase/1000;
    tc[i].trigHoldoff = 0;
    tc[i].traceOffset = tc[i].traceSize/2;
    tc[i].trigOffset = 0;
    tc[i].triggerEdge = true;
  }

  tc[0].color[0] = 0.13f;
  tc[0].color[1] = 0.97f;
  tc[0].color[2] = 0.21f;
  tc[0].color[3] = 0.95f;
  if (channels > 1) {
  tc[1].color[0] = 0.93f;
  tc[1].color[1] = 0.17f;
  tc[1].color[2] = 0.23f;
  tc[1].color[3] = 0.95f;
  }

  xyp.color[0] = 0.13f;
  xyp.color[1] = 0.97f;
  xyp.color[2] = 0.21f;
  xyp.color[3] = 0.95f;
  xyp.xOffset = 0;
  xyp.yOffset = 0;
  xyp.xScale = 1.0f;
  xyp.yScale = 1.0f;
  xyp.persistence = timebaseDef;
  xyp.sampleLen = sampleRate*xyp.persistence/1000;

  winC.w = 1280;
  winC.h = 720;
  winC.layout = 
"[Window][DockSpaceViewport_11111111]\n"
"Pos=0,0\n"
"Size=1280,720\n"
"Collapsed=0\n"
"\n"
"[Window][Debug##Default]\n"
"Pos=60,60\n"
"Size=400,400\n"
"Collapsed=0\n"
"\n"
"[Window][Controls]\n"
"Pos=725,0\n"
"Size=275,167\n"
"Collapsed=0\n"
"DockId=0x00000002,0\n"
"\n"
"[Window][Scope]\n"
"Pos=0,0\n"
"Size=918,498\n"
"Collapsed=0\n"
"DockId=0x00000009,0\n"
"\n"
"[Window][Scope (XY)]\n"
"Pos=920,0\n"
"Size=360,326\n"
"Collapsed=0\n"
"DockId=0x0000000B,0\n"
"\n"
"[Window][Global Controls]\n"
"Pos=982,500\n"
"Size=298,220\n"
"Collapsed=0\n"
"DockId=0x00000004,0\n"
"\n"
"[Window][Channel 1 Controls]\n"
"Pos=0,500\n"
"Size=481,220\n"
"Collapsed=0\n"
"DockId=0x00000007,0\n"
"\n"
"[Window][Channel 2 Controls]\n"
"Pos=483,500\n"
"Size=497,220\n"
"Collapsed=0\n"
"DockId=0x00000003,0\n"
"\n"
"[Window][XY Scope Controls]\n"
"Pos=920,328\n"
"Size=360,170\n"
"Collapsed=0\n"
"DockId=0x0000000C,0\n"
"\n"
"[Docking][Data]\n"
"DockSpace         ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,0 Size=1280,720 Split=X Selected=0x7C3EDFF1\n"
"  DockNode        ID=0x00000001 Parent=0x8B93E3BD SizeRef=578,371 Split=Y Selected=0x7C3EDFF1\n"
"    DockNode      ID=0x00000005 Parent=0x00000001 SizeRef=1003,498 Split=X Selected=0x7C3EDFF1\n"
"      DockNode    ID=0x00000009 Parent=0x00000005 SizeRef=918,511 CentralNode=1 Selected=0x7C3EDFF1\n"
"      DockNode    ID=0x0000000A Parent=0x00000005 SizeRef=360,511 Split=Y Selected=0x5D48DF31\n"
"        DockNode  ID=0x0000000B Parent=0x0000000A SizeRef=360,326 Selected=0x5D48DF31\n"
"        DockNode  ID=0x0000000C Parent=0x0000000A SizeRef=360,170 Selected=0x9828E27C\n"
"    DockNode      ID=0x00000006 Parent=0x00000001 SizeRef=1003,220 Split=X Selected=0x5E07700B\n"
"      DockNode    ID=0x00000007 Parent=0x00000006 SizeRef=481,207 Selected=0x5E07700B\n"
"      DockNode    ID=0x00000008 Parent=0x00000006 SizeRef=797,207 Split=X Selected=0x226655D0\n"
"        DockNode  ID=0x00000003 Parent=0x00000008 SizeRef=497,207 Selected=0x226655D0\n"
"        DockNode  ID=0x00000004 Parent=0x00000008 SizeRef=298,207 Selected=0xDFE49EEC\n"
"  DockNode        ID=0x00000002 Parent=0x8B93E3BD SizeRef=275,371 HiddenTabBar=1 Selected=0x67284010\n";

  oscDataSize = dataSize;

  oscData = new float*[channels];
  oscAuxData = new float*[channels];
  oscAlign = new float[oscDataSize];
  for (unsigned char i = 0; i < channels; i++) {
    oscData[i] = new float[oscDataSize];
    oscAuxData[i] = new float[oscDataSize];

    memset(oscData[i],0,oscDataSize*sizeof(float));
    memset(oscAuxData[i],0,oscDataSize*sizeof(float));
  }
  memset(oscAlign,0,oscDataSize*sizeof(float));
  running = false;
  updateOsc = true;
  restartAudio = true;
  audioLoopback = false;

  device = 0;
  deviceNum = 0;
  devs.clear();
  showTrigger = false;

  fullscreen = false;

  ai = NULL;
  ap = NULL;
}

void GUI::attachAudioInput(AudioInput *i) {
  ai = i;
}

void GUI::attachAudioProcess(AudioProcess *p) {
  ap = p;
}


int GUI::init() {
  if (running) return 0;
  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }
  // Decide GL+GLSL versions
  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  window = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winC.w, winC.h, window_flags);
  if (window == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return -1;
  }
  gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = ImGui::GetIO();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
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
  ImGui::LoadIniSettingsFromMemory(winC.layout);
  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);
  clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  running = true;
  return 0;
}

void GUI::getDevices(std::vector<DeviceEntry> d) {
  devs = d;
}

void GUI::doFrame() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT)
      running = false;
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
      running = false;
  }
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
  
  ImGui::DockSpaceOverViewport(ImGui::GetWindowViewport(),0);

  for (unsigned char j = 0; j < channels; j++) {
    ai->setAlignParams(AlignParams(tc[j].trigger,tc[j].traceSize,tc[j].traceOffset,tc[j].triggerEdge,tc[j].trigHoldoff));
    writeOscData(j,
      ai->getAlignRamp(j),
      ai->getData(j));
  }

  GUI::drawGUI();

  ImGui::Render();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  // Update and Render additional Platform Windows
  // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
  //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
      SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
      SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
  }
  SDL_GL_SwapWindow(window);
}

void GUI::drawGUI() {
  if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
    fullscreen = !fullscreen;
    SDL_SetWindowFullscreen(window,fullscreen?(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP):0);
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

  drawControls();
  ImPlot::CreateContext();
    drawMainScope();
    // drawAuxScope();
    drawXYScope();
  ImPlot::DestroyContext();
}

void GUI::writeOscData(unsigned char chan, float* datax, float* datay) {
  if (!updateOsc) return;
  memcpy(oscAlign,datax,oscDataSize*sizeof(float));
  memcpy(oscData[chan],datay,oscDataSize*sizeof(float));
}

void GUI::writeOscAuxData(unsigned char chan, float* data) {
  if (!updateOsc) return;
  memcpy(oscData[chan],data,oscDataSize*sizeof(float));
}

void GUI::audioSet() {
  restartAudio = false;
}

bool GUI::doRestartAudio() {
  return restartAudio;
}

int GUI::getAudioDeviceSetting() {
  return device;
}

void GUI::setAudioDeviceSetting(int d) {
  device = d;
  for (int i = 0; i < devs.size(); i++) {
    if (devs[i].dev == device) {
      deviceNum = i;
      break;
    }
  }
}

GUI::~GUI() {
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

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
    delete[] oscAlign;
    oscAlign = NULL;
  }
  ai = NULL;
  ap = NULL;

  if (tc) {
    delete[] tc;
    tc = NULL;
  }
}

const char* numberStrs[16] = {
  "0","1","2","3",
  "4","5","6","7",
  "8","9","10","11",
  "12","13","14","15"
};