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

#include "render_directx9.h"

int USCRenderDirectX9::initRender() {
  clearColor[0] = 0.f;
  clearColor[1] = 0.f;
  clearColor[2] = 0.f;
  clearColor[3] = 0.f;
  return 0;
}

int USCRenderDirectX9::setupRender(int _winFlags, const char* winName, int winX, int winY, int winW, int winH) {
  return 1; // :sob:
  // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
  // Setup window
  winFlags = _winFlags;

  win = SDL_CreateWindow(winName, winX, winY ,winW, winH, winFlags);
  if (win == NULL) {
    return 1;
  }

  // if (CreateDeviceD3D(0,D3DDEVTYPE_HAL,NULL)) {
  //   CleanupDeviceD3D();
  //   return 1;
  // }

  ImGui_ImplSDL2_InitForD3D(win);
  ImGui_ImplDX9_Init(dev);

  return 0;
}

int USCRenderDirectX9::renderPreLoop() {
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT)
      return -1;
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(win))
      return -1;
  }

  // Start the Dear ImGui frame
  ImGui_ImplDX9_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  return 0;
}

int USCRenderDirectX9::renderPostLoop() {
  dev->SetRenderState(D3DRS_ZENABLE, FALSE);
  dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  dev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
  D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clearColor[0]*clearColor[3]*255.0f), (int)(clearColor[1]*clearColor[3]*255.0f), (int)(clearColor[2]*clearColor[3]*255.0f), (int)(clearColor[3]*255.0f));
  dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
  if (dev->BeginScene() >= 0) {
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    dev->EndScene();
  }

  // Update and Render additional Platform Windows
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
  dev->EndScene();

  return 0;
}

void USCRenderDirectX9::destroyRender() {
  ImGui_ImplDX9_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  dev->Release();
  SDL_DestroyWindow(win);
  SDL_Quit();
}
