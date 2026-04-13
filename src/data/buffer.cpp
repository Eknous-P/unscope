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

#include "buffer.h"
#include <cassert>

DataBuffer::DataBuffer():
  size(0), index(0),
  sampleRate(0.0),
  name(""),
  inited(false) {}

void DataBuffer::init(nint len, double rate, string n) {
  size = len;
  sampleRate = rate;
  name = n;
  index = 0;
  inited = false;
  assert(0 && "you called the virtual constructor!");
}

void DataBuffer::write(void* sample) {
  (void)sample;
  if (++index>=size) index=0;
}

nint DataBuffer::getSize() const {
  return size;
}

nint DataBuffer::getIndex() {
  return index;
}

double DataBuffer::getSampleRate() const {
  return sampleRate;
}

void* DataBuffer::getBuffer() {
  return NULL;
}

string DataBuffer::getName() const {
  return name;
}

void DataBuffer::destroy() {
  inited = false;
}

double DataBuffer::getValueScaled(nint i) {
  return 0.0;
}

bool DataBuffer::isInited() {
  return inited;
}

YAML::Node DataBuffer::writeToNode() {
  YAML::Node node;
  node["inited"] = inited;
  if (inited) {
    node["size"] = size;
    node["sampleRate"] = sampleRate;
    node["name"] = name;
  }
  return node;
}

bool DataBuffer::readFromNode(YAML::Node node) {
  if (!node["inited"].IsDefined()) return false;
  inited = node["inited"].as<bool>();
  if (inited) init(
    node["size"].as<nint>(),
    node["sampleRate"].as<double>(),
    node["name"].as<string>()
  );
  return true;
}

DataBuffer::~DataBuffer() {
}


DataBuffer_Float::DataBuffer_Float():
  DataBuffer(),
  data(NULL) {}

void DataBuffer_Float::init(nint len, double rate, string n) {
  size = len;
  sampleRate = rate;
  name = n;
  index = 0;
  data = new float[size];
  inited = true;
}

void DataBuffer_Float::write(void* sample) {
  if (data==NULL) return;
  data[index++]=*(float*)sample;
  if (index>=size) index=0;
}

void* DataBuffer_Float::getBuffer() {
  return data;
}

void DataBuffer_Float::destroy() {
  if (data) {
    delete[] data;
    data = NULL;
  }
  inited = false;
}

double DataBuffer_Float::getValueScaled(nint i) {
  if (data==NULL) return 0;
  float s;
  s=data[(index+i)%size];
  return s;
}

DataBuffer_Float::~DataBuffer_Float() {
  destroy();
}
