#include "../gui.h"

int USCRender::initRender() {
  return 0;
}

int USCRender::setupRender(SDL_WindowFlags _winFlags, const char* winName, ImVec2 winPos, ImVec2 winSize) {
  win=NULL;
  winFlags=_winFlags;
  (void)winName;
  (void)winPos;
  (void)winSize;
  return 0;
}

int USCRender::renderPreLoop() {
  return 0;
}

int USCRender::renderPostLoop() {
  return 0;
}

void USCRender::destroyRender() {
}

SDL_Window* USCRender::getWindow() {
  return win;
}

USCRender::~USCRender() {
}
