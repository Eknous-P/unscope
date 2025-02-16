#include "../gui.h"

// TODO: rewrite render
// its soo messy

int USCRender::setup(int width, int height) {
  return 0;
}

int USCRender::init() {
  return 0;
}

bool USCRender::beginFrame() {
  return true;
}

void USCRender::endFrame(ImGuiIO io, ImVec4 col) {
}

void USCRender::deinit() {
}

SDL_Window* USCRender::getWindow() {
  return NULL;
}