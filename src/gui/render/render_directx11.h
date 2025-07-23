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

#ifndef USC_RENDER_DIRECTX_H
#define USC_RENDER_DIRECTX_H

#include "../gui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <SDL_syswm.h>

class USCRenderDirectX11 : public USCRender {
  SDL_Window* win;
  int winFlags;
  SDL_Event event;
  
  ID3D11Device* g_pd3dDevice;
  ID3D11DeviceContext* g_pd3dDeviceContext;
  IDXGISwapChain* g_pSwapChain;
  ID3D11RenderTargetView* g_mainRenderTargetView;
  HWND hwnd;
  WNDCLASSEXW wc;

  float clearColor[4];

  bool CreateDeviceD3D(HWND _hwnd);
  void CleanupDeviceD3D();
  void CreateRenderTarget();
  void CleanupRenderTarget();
  public:
    int initRender();
    int setupRender(int _winFlags, const char* winName, int winX, int winY, int winW, int winH);
    int renderPreLoop();
    int renderPostLoop();
    void destroyRender();
};

#endif
