#ifndef USCRENDERER_DIRECTX_SDL
#define USCRENDERER_DIRECTX_SDL

#include "../gui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <SDL.h>
#include <SDL_syswm.h>

class USCRenderDirectX11 : public USCRender {
  SDL_Window* win;
  SDL_WindowFlags winFlags;
  SDL_Event event;
  
  ID3D11Device* g_pd3dDevice;
  ID3D11DeviceContext* g_pd3dDeviceContext;
  IDXGISwapChain* g_pSwapChain;
  ID3D11RenderTargetView* g_mainRenderTargetView;
  HWND hwnd;
  WNDCLASSEXW wc;

  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();
  void CreateRenderTarget();
  void CleanupRenderTarget();
  public:
    int initRender();
    int setupRender(SDL_WindowFlags _winFlags, const char* winName, ImVec2 winPos, ImVec2 winSize);
    int renderPreLoop();
    int renderPostLoop();
    void destroyRender();
};

#endif