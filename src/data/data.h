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

/*
 * defines data i/o classes and stuff
 */

#ifndef USC_AUDIO_H
#define USC_AUDIO_H

#include "shared.h"
#include "param.h"
#include "buffer.h"

// the different formats a driver may support
enum DataFormat {
  DATAFORMAT_S8=0,
  DATAFORMAT_U8,
  DATAFORMAT_S16,
  DATAFORMAT_S32,
  DATAFORMAT_F32
};

// enum of the available drivers
enum DataDrivers {
  DATA_DUMMY=0,
  DATA_PORTAUDIO,
  DATA_SDL,
  DATA_PIPEWIRE,
  DATA_MAX
};

// type declaration (as its pointer is used below)
class DataDriver;

enum DataDriverCommands {
  DRIVER_ENUMERATE_DEVICES,
  DRIVER_ACTIVATE,
  DRIVER_PLAY,
  DRIVER_PAUSE,
  DRIVER_DEACTIVATE,
  DRIVER_DESTROY,
};

typedef int DataDriverState;

enum DataDriverStates : DataDriverState {
  DRIVERSTATE_NONE   = 0,
  DRIVERSTATE_OK     = 1u<<0,
  DRIVERSTATE_READY  = 1u<<1,
  DRIVERSTATE_ACTIVE = 1u<<2,
  DRIVERSTATE_PLAY   = 1u<<3,

  DRIVERSTATE_ERROR  = 1u<<30
};

/* main data handler.
 * stores the buffers and the drivers.
 * manages drivers and provides the buffers outside.
 */
class USCData {
  private:
    USCConfig* config;
    vector<DataDriver*> drivers;
  public:
    size_t getDriverCount();
    /** return the pointer to a specific driver
     */
    DataDriver* getDriver(size_t which);
    /** add a driver.
     * @param which which driver from the DataDrivers enum
     * @return the status of the operation (0 - success)
     */
    int addDriver(DataDrivers which);
    /** remove a driver
     * @param which the number of the driver to remove
     */
    int removeDriver(int which);
    /** send a command to a driver
     * @param driver the numerb of the driver
     * @param cmd the command to send
     */
    int dispatchDriverCommand(unsigned int driver, DataDriverCommands cmd);

    int loadFromConfig();
    int saveToConfig();

    USCData(USCConfig* conf);
    ~USCData();
};

enum DataDeviceFlags {
  DATADEVICEFLAG_NONE = 0,
  DATADEVICEFLAG_DIR_IN  = 1u<<0,
  DATADEVICEFLAG_DIR_OUT = 1u<<1,
};

class DataDevice {
  string name;
  int flags;
};

enum DataDriverFlags : unsigned int {
  DRIVERFLAG_NONE   = 0,
  DRIVERFLAG_OUTPUT = 1u<<0,
  DRIVERFLAG_INPUT  = 1u<<1,
  DRIVERFLAG_PAUSE  = 1u<<2,
};

struct DataDriverInfo {
  DataDrivers id;
  int flags;
  const char* name;
};

class DataDriver {
  protected:
    USCData* parent;
    vector<DataBuffer*> buffers;
    vector<Parameter> config;
    DataDriverState state;
    int error;

    YAML::Node saveBufferToNode();
    void loadBufferFromNode(YAML::Node node);
  public:
    virtual const DataDriverInfo     getDriverInfo()      const;
    virtual       DataBuffer*        getBuffer(int which) const;
    virtual       int                getBufferCount()     const;
    virtual       vector<Parameter>* getParams()               ;
    virtual       DataDriverState    getState()           const;
    virtual       int                getLastError();

    virtual int setup(USCData* p);
    virtual int enumerateDevices();

    virtual void activate();
    virtual void doPlay(bool play);
    virtual void deactivate();

    virtual void destroy();

    virtual YAML::Node saveToNode();
    virtual void loadFromNode(YAML::Node& node);
};

#endif
