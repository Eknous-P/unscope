#include "gui.h"

bool GUI::isRunning() {
  return running;
}

GUI::GUI(unsigned long int sampleRateDef, unsigned long int dataSize, unsigned char chanCount, float timebaseDef, float yScaleDef, float triggerDef) {
  err = 0;

  sampleRate = sampleRateDef;
  sc.timebase = timebaseDef;
  sc.traceSize = sampleRate*sc.timebase/125;
  sc.traceOffset = sc.traceSize/2;
  sc.yScale = yScaleDef;
  sc.trigger = 0;
  sc.triggerEdge = true;

  sc.color[0][0] = 0.13f;
  sc.color[0][1] = 0.97f;
  sc.color[0][2] = 0.21f;
  sc.color[0][3] = 0.95f;

  sc.color[1][0] = 0.93f;
  sc.color[1][1] = 0.17f;
  sc.color[1][2] = 0.23f;
  sc.color[1][3] = 0.95f;

  sc.color[2][0] = 0.87f;
  sc.color[2][1] = 0.93f;
  sc.color[2][2] = 0.14f;
  sc.color[2][3] = 0.95f;

  winC.w = 1000;
  winC.h = 430;
  winC.layout = 
"[Window][DockSpaceViewport_11111111]\n"
"Pos=0,0\n"
"Size=1000,421\n"
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
"DockId=0x00000003,0\n"
"\n"
"[Window][Scope]\n"
"Pos=0,0\n"
"Size=723,421\n"
"Collapsed=0\n"
"DockId=0x00000001,0\n"
"\n"
"[Window][Scope (XY)]\n"
"Pos=725,169\n"
"Size=275,252\n"
"Collapsed=0\n"
"DockId=0x00000004,0\n"
"\n"
"[Docking][Data]\n"
"DockSpace     ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,0 Size=1000,421 Split=X Selected=0x7C3EDFF1\n"
"  DockNode    ID=0x00000001 Parent=0x8B93E3BD SizeRef=578,371 CentralNode=1 HiddenTabBar=1 Selected=0x7C3EDFF1\n"
"  DockNode    ID=0x00000002 Parent=0x8B93E3BD SizeRef=275,371 Split=Y Selected=0x67284010\n"
"    DockNode  ID=0x00000003 Parent=0x00000002 SizeRef=275,167 HiddenTabBar=1 Selected=0x67284010\n"
"    DockNode  ID=0x00000004 Parent=0x00000002 SizeRef=275,252 HiddenTabBar=1 Selected=0x5D48DF31\n";


  channels = chanCount;
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

  triggerChan = 0;

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
  // ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
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
    writeOscData(j,
      ai->getAlignRamp(triggerChan,sc.trigger,sc.traceSize,sc.traceOffset,sc.triggerEdge),
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

  ImGui::Begin("Controls");
  ImGui::Checkbox("update",&updateOsc);
  sc.timebase = sc.traceSize * 125 / sampleRate;
  if (ImGui::SliderFloat("timebase", &sc.timebase, 0, (float)oscDataSize/(float)sampleRate*125, "%g ms/div")) {
    sc.traceSize = sampleRate * sc.timebase / 125;
    sc.traceOffset = ((sc.trigOffset + 1.0f)/2) * sc.traceSize;
    if (sc.traceOffset + sc.traceSize > oscDataSize) sc.traceOffset = oscDataSize - sc.traceSize;
  }
  ImGui::SliderFloat("scale", &sc.yScale, 0.25f, 2.0f, "%g");
  ImGui::SliderFloat("trigger", &sc.trigger, -1.0f, 1.0f, "%g");
  showTrigger = ImGui::IsItemActive();
  showTrigger |= ai->didTrigger();
  if (sc.traceSize != 0) {
    sc.trigOffset = 2*((float)sc.traceOffset/(float)sc.traceSize)-1.0f;
  } else {
    sc.trigOffset = 0;
  }
  ImGui::SliderFloat("offset", &sc.trigOffset, -1.0f, 1.0f, "%g");
  if (ImGui::IsItemActive()) {
    sc.traceOffset = ((sc.trigOffset + 1.0f)/2) * sc.traceSize;
  }
  if (sc.traceOffset + sc.traceSize > oscDataSize) sc.traceOffset = oscDataSize - sc.traceSize;
  ImGui::AlignTextToFramePadding();
  ImGui::Text("trigger edge:");
  ImGui::SameLine();
  if (ImGui::Button(sc.triggerEdge?"Rising":"Falling")) sc.triggerEdge = !sc.triggerEdge;
  if (channels > 1) {
    if (ImGui::BeginCombo("trigger channel",numberStrs[triggerChan],ImGuiComboFlags_WidthFitPreview)) {
      for (unsigned char i = 0; i < channels; i++) {
        if (ImGui::Selectable(numberStrs[i],i == triggerChan)) triggerChan = i;
      }
      ImGui::EndCombo();
    }
  }

  if (ImGui::TreeNode("colors")) {
    for (unsigned char i = 0; i < channels; i++) {
      if (ImGui::TreeNode((void*)(intptr_t)i,"channel %d",i)) {
        ImGui::ColorPicker4("##color",sc.color[i]);
        ImGui::TreePop();
      }
    }
    ImGui::TreePop();
  }

  if (devs.size() > 0) {
    if (ImGui::BeginCombo("device",devs[deviceNum].devName.c_str())) {
      for (int i = 0; i < devs.size(); i++) {
        if (ImGui::Selectable(devs[i].devName.c_str(), deviceNum == i)) {
          deviceNum = i;
          device = devs[i].dev;
        }
      }
      ImGui::EndCombo();
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("R")) getDevices(ai->enumerateDevs());
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("refresh audio device list");
  }

  if (ImGui::Button("restart audio")) {
    err = ai->stop();
    printf("opening device %d: %s ...\n",device,Pa_GetDeviceInfo(device)->name);
    err = ai->init(device);
    if (err != paNoError) {
      printf("%d:%s", err, getErrorMsg(err).c_str());
      // try again
      if (err != paInvalidDevice) throw err;
      printf("trying default device...\n");
      device = Pa_GetDefaultInputDevice();
      err = ai->init(device);
      if (err != paNoError) {
        printf("%d:%s", err, getErrorMsg(err).c_str());
        throw err;
      }
      setAudioDeviceSetting(device);
    }
  }

  ImGui::End();
  ImGui::ShowDemoWindow();
  // ImGui::Begin("TEST",NULL,ImGuiWindowFlags_NoTitleBar);
  // GUI::startCRT(0xee444444, 10);
  // GUI::drawCRTLine(oscData[0]+(oscDataSize-sc.traceSize),oscData[1]+(oscDataSize-sc.traceSize),sc.traceSize,0xee00ff00,1.0f,0,0,1.0f);
  // ImGui::End();
    GUI::drawMainScope();
    // GUI::drawAuxScope();
    GUI::drawXYScope();
}

void GUI::startCRT(ImU32 gridColor, unsigned char gridSub) {
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 winSize = ImGui::GetWindowSize();
  ImVec2 origin = ImGui::GetWindowPos();
  unsigned int i = 0;
  float gridOffset = 0;
  dl->AddRectFilled(origin, ImVec2(origin.x+winSize.x, origin.y+winSize.y),ImU32(0xee111111));
  for (i = 0; i < gridSub+1; i++) {
    gridOffset = winSize.x/gridSub;
    dl->AddLine(ImVec2(origin.x+gridOffset*i,origin.y),ImVec2(origin.x+gridOffset*i,origin.y+winSize.y),gridColor,.5f);
    gridOffset = winSize.y/gridSub;
    dl->AddLine(ImVec2(origin.x,gridOffset*i+origin.y),ImVec2(origin.x+winSize.x,origin.y+gridOffset*i),gridColor,.5f);
  }
}

void GUI::drawCRTLine(float* Xdata, float* Ydata, unsigned long int length, ImU32 lineColor, float intensity, float xOffset, float yOffset, float xscale, float yscale, unsigned long int visibleLen) {
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 winSize = ImGui::GetWindowSize();
  ImVec2 center = ImVec2(winSize.x/2,winSize.y/2);
  ImVec2 origin = ImGui::GetWindowPos();
  unsigned int i = 0, step = 4;
  long int dataOffset = length - 2*visibleLen;
  if (dataOffset < 0) dataOffset = 0;
  if (oscDataSize/(visibleLen+1)>160) step = 2;
  if (oscDataSize/(visibleLen+1)>320) step = 1;
  for (i = step+dataOffset; i < length; i+=step) {
    dl->AddLine(ImVec2(clampF(center.x+origin.x+xscale*center.x*Xdata[i-step]+xOffset, origin.x, origin.x+winSize.x),clampF(center.y+origin.y-yscale*center.y*Ydata[i-step]+yOffset, origin.y, origin.y+winSize.y)),
                ImVec2(clampF(center.x+origin.x+xscale*center.x*Xdata[i]+xOffset, origin.x, origin.x+winSize.x),clampF(center.y+origin.y-yscale*center.y*Ydata[i]+yOffset, origin.y, origin.y+winSize.y)),
                lineColor,intensity);
  }
}

void GUI::drawMainScope() {
  ImGui::Begin("Scope");
    GUI::startCRT(0xee444444, 8);
    for (unsigned char i = 0; i < channels; i++) {
      if (oscAlign[i] && oscData[i] && oscDataSize)
        GUI::drawCRTLine(oscAlign,oscData[i],oscDataSize,ImGui::ColorConvertFloat4ToU32(ImVec4(sc.color[i][0],sc.color[i][1],sc.color[i][2],sc.color[i][3])),1.0f,0,0,1.0f,sc.yScale,sc.traceSize);
    }

    ImVec4 trigColor = ai->didTrigger()?ImVec4(0,1,0,.5f):ImVec4(1,0,0,.5f);
    double trigDouble = sc.trigger;

  ImGui::End();
}

void GUI::drawAuxScope() {
  ImGui::Begin("Scope (Auxiliary)");
  ImGui::End();
}

void GUI::drawXYScope() {
  if (channels < 2) return;
  ImGui::Begin("Scope (XY)");
    GUI::startCRT(0xee444444, 8);
    GUI::drawCRTLine(oscData[0]+(oscDataSize-sc.traceSize),oscData[1]+(oscDataSize-sc.traceSize),sc.traceSize,ImGui::ColorConvertFloat4ToU32(ImVec4(sc.color[0][0],sc.color[0][1],sc.color[0][2],sc.color[0][3])),.125f,0,0,sc.yScale,sc.yScale,sc.traceSize);

  ImGui::End();
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

unsigned long int GUI::getTraceSize() {
  return sc.traceSize;
}

float GUI::getTrigger() {
  return sc.trigger;
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
}

const char* numberStrs[16] = {
  "0","1","2","3",
  "4","5","6","7",
  "8","9","10","11",
  "12","13","14","15"
};