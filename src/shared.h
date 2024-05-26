#ifndef SHARED_H
#define SHARED_H

#include <vector>
#include <string>

#define PROGRAM_NAME "unscope"
#define PROGRAM_VER "0"

enum AudioErrors {
  NOERR=0,
  NOINIT,
  NODEVS,
  NODEV,
  NOSTART,
  NOGOOD
};

enum unscopeParamFlagss {
  UPARAM_HELP = 'h',
  UPARAM_BUFFERSIZE = 'b',
  UPARAM_FRAMESIZE = 'f',
  UPARAM_CHANNELCOUNT = 'c',
  UPARAM_SAMPLERATE = 's',
  UPARAM_VERSION = 'v'
};

struct DeviceEntry {
  int dev;
  std::string devName;
  DeviceEntry(int d, std::string dn) {
    dev = d;
    devName = dn;
  }
};

extern const char *helpMsg, *verMsg, *errMsgs[];

#endif