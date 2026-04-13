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

#ifdef USE_PIPEWIRE_DEFINE

#include "data.h"
#include <cstdint>
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>

class DataPipeWire;

struct pwCallbackData {
  DataPipeWire* parent;
  struct pw_thread_loop* pwLoop;
  struct pw_stream* stream;
};

struct pwDevice {
  string name;
  string desc;
};

class DataPipeWire : public DataDriver {
  pwCallbackData callbackData;

  struct spa_pod* streamParams[1];
  struct pw_properties* pwProps;
  struct spa_pod_builder spaBuilder;
  unsigned char builderBuf[1024];

  static void pwProcessCallback(void* userdata);
  static void pwHandleExitSignal(void* userdata, int signal);
  // holy fuck of a fuction to *enumerate deivces*!!!
  static void registryCallback(void* data, uint32_t id, uint32_t permissions, const char* type, uint32_t version, const struct spa_dict* props);

  vector<string> devices;
  vector<pwDevice> devicesInternal;
  typedef std::pair<vector<string>*,vector<pwDevice>*> deviceEnumCallbackData;
  deviceEnumCallbackData devicesVectorsPointers;

  int deviceNum;
  int channels;
  int sampleRateNum;

  int e;
  public:
    const DataDriverInfo getDriverInfo() const;
    int setup(USCData* p);
    int enumerateDevices();
    void activate();
    void doPlay(bool play);
    void deactivate();
    void destroy();
    // int getLastError();
    ~DataPipeWire();
};

#endif
