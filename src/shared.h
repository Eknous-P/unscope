#ifndef SHARED_H
#define SHARED_H

#include <vector>

struct DeviceEntry {
  int dev;
  const char* devName;
  DeviceEntry(int d, const char* dn) {
    dev = d;
    devName = dn;
  }
};

#endif