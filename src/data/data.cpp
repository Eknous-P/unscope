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
// #include "driver/dummy.h"
#include "driver/data_portaudio.h"
#include "driver/data_sdl.h"
#ifdef USE_PIPEWIRE_DEFINE
#include "driver/data_pipewire.h"
#endif

USCData::USCData(USCConfig* conf) {
  config = conf;
  drivers.clear();
}

size_t USCData::getDriverCount() {
  return drivers.size();
}

DataDriver* USCData::getDriver(size_t which) {
  if (which>drivers.size()-1) return NULL;
  return drivers[which];
}

int USCData::addDriver(DataDrivers which) {
  DataDriver* drv=NULL;
  switch (which) {
    case DATA_PORTAUDIO:
      drv=new DataPortAudio;
      break;
    case DATA_SDL:
      drv=new DataSDL;
      break;
#ifdef USE_PIPEWIRE_DEFINE
    case DATA_PIPEWIRE:
      drv=new DataPipeWire;
      break;
#endif
    case DATA_DUMMY:
    default: break;
  }
  if (drv==NULL) return 1;
  if (drv->setup(this)) {
    printf(ERROR_MSG "Data: failed to initialize driver!" MSG_END);
    return 1;
  }
  drivers.push_back(drv);
  return 0;
}

int USCData::removeDriver(int which) {
  drivers[which]->destroy();
  delete drivers[which];
  auto drv = drivers.begin();
  std::advance(drv,which);
  drivers.erase(drv);
  return 0;
}

int USCData::dispatchDriverCommand(unsigned int driver, DataDriverCommands cmd) {
  if (driver>drivers.size()-1) return -1;
  DataDriver* drv = drivers[driver];
  switch (cmd) {
    case DRIVER_ENUMERATE_DEVICES:
      return drv->enumerateDevices();
    case DRIVER_ACTIVATE:
      drv->activate();
      return drv->getLastError();
    case DRIVER_PLAY:
      drv->doPlay(true);
      return drv->getLastError();
    case DRIVER_PAUSE:
      drv->doPlay(false);
      return drv->getLastError();
    case DRIVER_DEACTIVATE:
      drv->deactivate();
      return drv->getLastError();
    case DRIVER_DESTROY:
      drv->destroy();
      return drv->getLastError();
    default: return -1;
  }
}

int USCData::loadFromConfig() {
  char strbuf[256];
  snprintf(strbuf, 256, "dataDrv0");
  for (int i=0; config->exists(strbuf); i++) {
    YAML::Node node = config->getConfig<YAML::Node>(strbuf, YAML::Node());
    if (addDriver((DataDrivers)node["id"].as<int>()) == 0) {
      YAML::Node drvData = node["data"];
      drivers.back()->enumerateDevices();
      drivers.back()->loadFromNode(drvData);
    }
    snprintf(strbuf, 256, "dataDrv%d", i+1);
  }
  return 0;
}

int USCData::saveToConfig() {
  char strbuf[256];
  for (int i=0; i<drivers.size(); i++) {
    DataDriver* drv = drivers[i];
    YAML::Node node;
    snprintf(strbuf, 256, "dataDrv%d", i);
    node["id"] = (int)drv->getDriverInfo().id;
    node["data"] = drv->saveToNode();
    config->setConfig(strbuf, node);
  }
  return 0;
}

USCData::~USCData() {
  for (DataDriver*& drv:drivers) {
    drv->destroy();
    delete drv;
  }
  drivers.clear();
}
