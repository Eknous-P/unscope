#include "../gui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_syswm.h>

#ifndef USCRENDERER_DIRECTX_SDL
#define USCRENDERER_DIRECTX_SDL

class USCRendDirectX : public USCRender {
  private:
    ID3D11Device* g_pd3dDevice;
    ID3D11DeviceContext* g_pd3dDeviceContext;
    IDXGISwapChain* g_pSwapChain;
    ID3D11RenderTargetView* g_mainRenderTargetView;
    SDL_Window* window;
    SDL_WindowFlags window_flags;
    SDL_Event event;
    HWND hwnd;
    WNDCLASSEXW wc;

    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();
  public:
    int setup();
    int init();
    bool beginFrame();
    void endFrame(ImGuiIO io, ImVec4 col);
    void deinit();
    void doFullscreen(bool f);
};

#endif