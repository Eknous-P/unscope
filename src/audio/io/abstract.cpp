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

#include "audio.h"

int AudioIO::setup(USCAudio* p) {
  parent = p;
  conf = p->getConfig();
  buf  = p->getBuffer();
  devs = p->getDevices();
  callbackFunction = p->getCallbackFunction();

  running = false;
  outputting = false;
  return 0;
}

int AudioIO::init() {
  return 0;
}

int AudioIO::deinit() {
  return 0;
}

int AudioIO::start() {
  running = true;
  return 0;
}

int AudioIO::stop() {
  running = false;
  return 0;
}

bool AudioIO::isRunning() {
  return running;
}

bool AudioIO::isOutputting() {
  return outputting;
}

int AudioIO::getAvailDevices() {
  return 0;
}

int AudioIO::getDefaultInputDevice() {
  return 0;
}

int AudioIO::getDefaultOutputDevice() {
  return 0;
}

const char* AudioIO::getLastError() {
  return NULL;
}
