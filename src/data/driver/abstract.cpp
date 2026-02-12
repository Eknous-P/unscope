/*
unscope - an audio oscilloscope
Copyright (C) 2025-2026 Eknous

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

#include "data.h"
#include <param.h>

const int DataDriver::getFlags() {
  return DRIVERFLAG_NONE;
}

int DataDriver::setup(USCData* p) {
  parent         = p;
  running        = false;
  lastErrorStr   = "";
  return 0;
}

int DataDriver::init() {
  return 0;
}

int DataDriver::deinit() {
  buffers = {};
  return 0;
}

int DataDriver::start() {
  running = true;
  return 0;
}

int DataDriver::stop() {
  running = false;
  return 0;
}

bool DataDriver::isRunning() {
  return running;
}

int DataDriver::enumerateDevices() {
  return 0;
}

int DataDriver::getDefaultInputDevice() {
  return 0;
}

int DataDriver::getDefaultOutputDevice() {
  return 0;
}

string DataDriver::getLastError() {
  return lastErrorStr;
}

const char* DataDriver::getName() {
  return "";
}

DataBuffer* DataDriver::getBuffer(int which) {
  if (which < 0 || which > buffers.size()) return NULL;
  return buffers[which];
}

int DataDriver::getBufferCount() {
  return buffers.size();
}

vector<Parameter> DataDriver::getParams() {
  return config;
}

void DataDriver::destroyParams() {
  for (Parameter& i:config) i.destroy();
  config.clear();
}
