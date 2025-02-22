#include "render_opengl2.h"
#include <SDL_video.h>

int USCRenderOpenGL2::initRender() {
  return 0;
}

int USCRenderOpenGL2::setupRender(int _winFlags, const char* winName, int winX, int winY, int winW, int winH) {
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  winFlags = _winFlags|SDL_WINDOW_OPENGL;
  win=SDL_CreateWindow(winName, winX, winY, winW, winH, winFlags);
  if (win==NULL) {
    return 1;
  }

  glContext=SDL_GL_CreateContext(win);
  SDL_GL_MakeCurrent(win, glContext);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL2_InitForOpenGL(win, glContext);
  ImGui_ImplOpenGL2_Init();
  return 0;
}

int USCRenderOpenGL2::renderPreLoop() {
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type==SDL_QUIT)
      return -1;
    if (event.type==SDL_WINDOWEVENT && event.window.event==SDL_WINDOWEVENT_CLOSE && event.window.windowID==SDL_GetWindowID(win))
      return -1;
  }
  if (SDL_GetWindowFlags(win) & SDL_WINDOW_MINIMIZED) {
    SDL_Delay(10);
    return 1; // continue
  }

  ImGui_ImplOpenGL2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  return 0;
}

int USCRenderOpenGL2::renderPostLoop() {
  ImGuiIO io=ImGui::GetIO();
  ImGui::Render();
  glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)io.DisplaySize.y);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
    SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
  }

  SDL_GL_SwapWindow(win);
  return 0;
}

void USCRenderOpenGL2::destroyRender() {
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(win);
  SDL_Quit();
}

SDL_Window* USCRenderOpenGL2::getWindow() {
  return win;
}


USCRenderOpenGL2::~USCRenderOpenGL2() {
}
