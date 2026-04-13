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

YAML::Node DataDriver::saveBufferToNode() {
  YAML::Node node;
  for (DataBuffer*& buf:buffers)
    node.push_back(buf->writeToNode());
  return node;
}

void DataDriver::loadBufferFromNode(YAML::Node node) {
  for (int i=0; i<buffers.size(); i++) {
    buffers[i]->readFromNode(node[i]);
  }
}

const DataDriverInfo DataDriver::getDriverInfo() const {
  return {
    DATA_DUMMY,
    DRIVERFLAG_NONE,
    "",
  };
}

DataBuffer* DataDriver::getBuffer(int which) const {
  if (which < 0 || which > buffers.size()) return NULL;
  return buffers[which];
}

int DataDriver::getBufferCount() const {
  return buffers.size();
}

vector<Parameter>* DataDriver::getParams() {
  return &config;
}

int DataDriver::getLastError() {
  state &=~DRIVERSTATE_ERROR;
  int oldError = error;
  error = 0;
  return oldError;
}

int DataDriver::setup(USCData* p) {
  parent      = p;
  state       = 0;
  error       = 0;
  return 0;
}

void DataDriver::activate() {
  state |= DRIVERSTATE_ACTIVE;
}

void DataDriver::doPlay(bool play) {
  if (play) {
    state |= DRIVERSTATE_PLAY;
  } else {
    state &=~DRIVERSTATE_PLAY;
  }
}

void DataDriver::deactivate() {
  state &=~DRIVERSTATE_ACTIVE;
}

DataDriverState DataDriver::getState() const {
  return state;
}

int DataDriver::enumerateDevices() {
  return 0;
}

void DataDriver::destroy() {
  for (Parameter& i:config) i.destroy();
  config.clear();
}

YAML::Node DataDriver::saveToNode() {
  YAML::Node node;
  node["state"] = (int)(state & ~(DRIVERSTATE_OK|DRIVERSTATE_READY));
  YAML::Node configNode;
  for (Parameter& p:config)
    p.writeToConfig(configNode);
  node["config"] = configNode;
  node["buffers"] = saveBufferToNode();
  return node;
}

void DataDriver::loadFromNode(YAML::Node& node) {
  YAML::Node configNode = node["config"];
  for (Parameter& p:config)
    p.readFromConfig(configNode);
  loadBufferFromNode(node["buffers"]);
  int tempState = node["state"].as<int>();
  state = (state&(DRIVERSTATE_OK|DRIVERSTATE_READY)) | tempState;
  // if (state&DRIVERSTATE_OK) {
    if (state&DRIVERSTATE_ACTIVE) {
      state&=~DRIVERSTATE_ACTIVE;
      activate();
    }
    if (state&DRIVERSTATE_PLAY) {
      state&=~DRIVERSTATE_PLAY;
      doPlay(true);
    }
  // }
}
