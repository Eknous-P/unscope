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
#include "driver/dummy.h"
#include "driver/data_portaudio.h"
#include "driver/data_sdl.h"

USCData::USCData() {
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
    case DATA_DUMMY:
      drv=new DummyDriver;
      break;
    case DATA_PORTAUDIO:
      drv=new DataPortAudio;
      break;
    case DATA_SDL:
      drv=new DataSDL;
      break;
    default: break;
  }
  if (drv==NULL) return 1;
  drv->setup(this);
  drivers.push_back(drv);
  return 0;
}

int USCData::removeDriver(int which) {
  drivers[which]->stop();
  drivers[which]->deinit();
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
    case DRIVER_INITIALIZE:
      return drv->init();
    case DRIVER_START_CALLBACK:
      return drv->start();
    case DRIVER_STOP_CALLBACK:
      return drv->stop();
    case DRIVER_RESTART:
      if (drv->isRunning()) drv->stop();
      drv->init();
      return drv->start();
    default: return -1;
  }
}

USCData::~USCData() {
  for (DataDriver*& drv:drivers) {
    drv->stop();
    drv->deinit();
    delete drv;
  }
  drivers.clear();
}
