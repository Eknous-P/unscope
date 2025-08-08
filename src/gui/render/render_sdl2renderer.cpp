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

#include "render_sdl2renderer.h"
#include <SDL_render.h>
#include <SDL_timer.h>
#include <SDL_video.h>

int USCRenderSDLRenderer::initRender() {
  return 0;
}

int USCRenderSDLRenderer::setupRender(int _winFlags, const char* winName, int winX, int winY, int winW, int winH) {
  winFlags = _winFlags;
  SDL_CreateWindowAndRenderer(winW, winH, _winFlags, &win, &renderer);
  SDL_SetWindowTitle(win, winName);
  if (!win || !renderer) return 1;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL2_InitForSDLRenderer(win, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);
  return 0;
}

int USCRenderSDLRenderer::renderPreLoop() {
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type==SDL_QUIT)
      return -1;
    if (event.type==SDL_WINDOWEVENT && event.window.event==SDL_WINDOWEVENT_CLOSE && event.window.windowID==SDL_GetWindowID(win))
      return -1;
  }
  if (SDL_GetWindowFlags(win) & SDL_WINDOW_MINIMIZED) {
    return 1; // continue
  }

  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  return 0;
}

int USCRenderSDLRenderer::renderPostLoop() {
  ImGuiIO io=ImGui::GetIO();
  ImGui::Render();
  SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
  SDL_RenderClear(renderer);
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);

  SDL_Delay(1);
  return 0;
}

void USCRenderSDLRenderer::destroyRender() {
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(win);
  SDL_Quit();
}

SDL_Window* USCRenderSDLRenderer::getWindow() {
  return win;
}


USCRenderSDLRenderer::~USCRenderSDLRenderer() {
}
