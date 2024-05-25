#include "gui.h"

bool GUI::isRunning() {
  return running;
}

GUI::GUI(unsigned long int dataSize, unsigned char chanCount, int traceSizeDef, float yScaleDef, float triggerDef) {
  sc.traceSize = traceSizeDef;
  sc.traceOffset = 0;
  sc.yScale = yScaleDef;
  sc.trigger = 0;

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

  sc.color[3][0] = 0.63f;
  sc.color[3][1] = 0.17f;
  sc.color[3][2] = 0.94f;
  sc.color[3][3] = 0.95f;

  winC.w = 1000;
  winC.h = 360;
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
"Pos=1000,0\n"
"Size=275,720\n"
"Collapsed=0\n"
"DockId=0x00000002,0\n"
"\n"
"[Window][Scope]\n"
"Pos=0,0\n"
"Size=1000,720\n"
"Collapsed=0\n"
"DockId=0x00000001,0\n"
"\n"
"[Docking][Data]\n"
"DockSpace   ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,0 Size=1280,720 Split=X Selected=0x7C3EDFF1\n"
"  DockNode  ID=0x00000001 Parent=0x8B93E3BD SizeRef=578,371 CentralNode=1 HiddenTabBar=1 Selected=0x7C3EDFF1\n"
"  DockNode  ID=0x00000002 Parent=0x8B93E3BD SizeRef=275,371 HiddenTabBar=1 Selected=0x67284010\n";

  channels = chanCount;
  oscDataSize = dataSize;

  oscData = new float*[channels];
  oscAuxData = new float*[channels];
  oscAlign = new float*[channels];
  for (unsigned char i = 0; i < channels; i++) {
    oscData[i] = new float[oscDataSize];
    oscAuxData[i] = new float[oscDataSize];
  }
  running = false;
  updateOsc = true;
  restartAudio = true;

  devs.clear();
  showTrigger = false;
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
  ImGui::Begin("Controls");
  ImGui::Checkbox("update",&updateOsc);
  ImGui::SliderInt("size", &sc.traceSize, 0, oscDataSize/channels, "%d");
  ImGui::SliderFloat("scale", &sc.yScale, 0.25f, 10.0f, "%f");
  ImGui::SliderFloat("trigger", &sc.trigger, -1.0f, 1.0f, "%f");
  showTrigger = ImGui::IsItemHovered();

  if (ImGui::TreeNode("color")) {
    ImGui::ColorPicker4("##color",sc.color[0]);
    ImGui::TreePop();
  }

  // if (ImGui::BeginCombo("device",devs[device].devName.c_str())) {
  //   for (int i = 0; i < devs.size(); i++) {
  //     if (ImGui::Selectable(devs[i].devName.c_str(), device == i)) {
  //       device = i;
  //     }
  //   }
  //   ImGui::EndCombo();
  // }

  if (ImGui::Button("restart audio")) restartAudio = true;

  ImGui::End();
  ImPlot::CreateContext();
    GUI::drawMainScope();
    // GUI::drawAuxScope();
  ImPlot::DestroyContext();
  // ImGui::Begin("Trigger");
  // ImGui::PlotLines("##trigger",oscAlign,65536,0,"",-1,1,ImGui::GetContentRegionAvail());
  // ImGui::End();
}

void GUI::drawMainScope() {
  unsigned char i = 0;
  ImGui::Begin("Scope");
  if (ImPlot::BeginPlot("##scope", ImGui::GetContentRegionAvail())) {
    for (i = 0; i < channels; i++) {
      ImPlot::SetupAxis(ImAxis(i),"t",ImPlotAxisFlags_NoDecorations);
      ImPlot::SetupAxis(ImAxis(i+3),"v",ImPlotAxisFlags_NoDecorations);
      ImPlot::SetupAxisLimits(ImAxis(i),-1, 1);
      ImPlot::SetupAxisLimits(ImAxis(i+3),-1.0f/sc.yScale,1.0f/sc.yScale);
    }
      ImPlot::SetupAxis(ImAxis_Y1,"##v",0);
    for (i = 0; i < channels; i++) {
      ImPlot::SetNextLineStyle(ImVec4(sc.color[i][0],sc.color[i][1],sc.color[i][2],sc.color[i][3]),0.25f);
      ImPlot::PlotLine("##scopeplot", oscAlign[i], oscData[i], oscDataSize,ImPlotFlags_NoLegend, 0);
    }
    // ImPlot::DragLineY(-1,(double*)&sc.trigger,ImVec4(1,0,0,1),1,ImPlotDragToolFlags_NoFit);
    if (showTrigger) ImPlot::TagY(sc.trigger,ImVec4(1,0,0,1),"trig");
    // ImPlot::TagYV()
    ImPlot::EndPlot();
  }
  ImGui::End();
}

void GUI::drawAuxScope() {
  ImGui::Begin("Scope (Auxiliary)");
  if (ImPlot::BeginPlot("##scope", ImGui::GetContentRegionAvail())) {
    ImPlot::SetupAxes("t","##v",ImPlotAxisFlags_NoDecorations,0);
    ImPlot::SetupAxisLimits(ImAxis_X1,(float)(oscDataSize - sc.traceSize)/oscDataSize, 1);
    ImPlot::SetupAxisLimits(ImAxis_Y1,-1.0f/sc.yScale,1.0f/sc.yScale);
    ImPlot::PlotLine("##scopeplot", oscAlign[0], oscAuxData[0], oscDataSize,ImPlotFlags_NoLegend, 0);
    ImPlot::EndPlot();
  }
  ImGui::End();
}


void GUI::writeOscData(unsigned char chan, float* datax, float* datay) {
  if (!updateOsc) return;
  oscAlign[chan] = datax;
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
    delete[] oscData;
    oscData = NULL;
  }
  if (oscAuxData) {
    delete[] oscAuxData;
    oscAuxData = NULL;
  }
}
