#include "../gui.h"

int USCRender::initRender() {
  return 0;
}

int USCRender::setupRender(int _winFlags, const char* winName, int winX, int winY, int winW, int winH) {
  win=NULL;
  winFlags=_winFlags;
  (void)winName;
  (void)winX;
  (void)winY;
  (void)winW;
  (void)winH;
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
