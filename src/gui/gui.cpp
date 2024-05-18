#include "gui.h"

bool GUI::isRunning() {
	return running;
}

GUI::GUI() {
  sc.traceSize=800;
  sc.traceOffset=0;
  sc.yScale=1.0f;
  running = false;
  oscData = new float[65536];
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
  window = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
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
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
  //io.ConfigViewportsNoAutoMerge = true;
  //io.ConfigViewportsNoTaskBarIcon = true;
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();
  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
  style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);
  clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  running = true;
	return 0;
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
  ImGui::DockSpaceOverViewport(NULL,0);

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
	ImGui::Begin("test");
  ImGui::SliderInt("size", &sc.traceSize, 0, oscDataSize, "%d");
  ImGui::SliderFloat("scale", &sc.yScale, 0.25f, 10.0f, "%f");
  ImGui::SliderFloat("trigger", &sc.trigger, -1.0f, 1.0f, "%f");

	ImGui::End();
  GUI::drawMainScope();
  // GUI::drawAuxScope();
}

void GUI::drawMainScope() {
  ImGui::Begin("Scope");
  ImPlot::CreateContext();
  if (ImPlot::BeginPlot("##scope")) {
    ImPlot::SetupAxes("t","##v",ImPlotAxisFlags_NoDecorations,0);
    ImPlot::SetupAxisLimits(ImAxis_X1,(float)(oscDataSize - sc.traceSize)/oscDataSize, 1);
    ImPlot::SetupAxisLimits(ImAxis_Y1,-1.0f/sc.yScale,1.0f/sc.yScale);
    ImPlot::PlotLine("##scopeplot", oscAlign, oscData, oscDataSize,ImPlotFlags_NoLegend, 0);
    ImPlot::EndPlot();
  }
  ImPlot::DestroyContext();
  ImGui::End();
}

void GUI::drawAuxScope() {
  ImGui::Begin("Scope (Auxiliary)");
  ImGui::PlotLines("",oscAuxData,sc.traceSize,0,NULL,-1.0f/sc.yScale,1.0f/sc.yScale,ImGui::GetContentRegionAvail());
  // ImPlot::CreateContext();
  // ImPlot::ShowDemoWindow();
  // ImPlot::PlotLine("");
  // ImPlot::DestroyContext();
  ImGui::End();
}


void GUI::writeOscData(float* datax, float* datay, unsigned int size) {
  oscAlign = datax;
  memcpy(oscData,datay,oscDataSize*sizeof(float));
  oscDataSize=size;
}

void GUI::writeOscAuxData(float* data, unsigned int size) {
  oscAuxData=data;
  oscAuxDataSize=size;
}

unsigned long int GUI::getTraceSize() {
  return sc.traceSize;
}

float GUI::getTrigger() {
  return sc.trigger;
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
}
