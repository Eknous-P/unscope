#ifndef SHARED_H
#define SHARED_H

#include <vector>
#include <string>

struct DeviceEntry {
  int dev;
  std::string devName;
  DeviceEntry(int d, std::string dn) {
    dev = d;
    devName = dn;
  }
};

#endif