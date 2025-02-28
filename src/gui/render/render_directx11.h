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