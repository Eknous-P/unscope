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
#include "portaudio.h"

class DataPortAudio : public DataDriver {
  PaStream* stream;
  PaError e;
  PaStreamParameters streamParamsI, streamParamsO;
  vector<string> inputDevices, outputDevices;
  vector<int> inputDevicesInternal, outputDevicesInternal;
  int inputChannels, outputChannels;
  bool outputting, paInitSuccess;

  int inputChannelsP, outputChannelsP;
  int inputDeviceP, outputDeviceP;
  int sampleRateP;
  int frameSizeP;

  PaError openStream(int iDev, int oDev, int iChans, int oChans, int sampleRate, int frames);

  static int audioCallback(
    const void *inputBuffer, void *outputBuffer,
    unsigned long int framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData);
  public:
    const DataDriverInfo getDriverInfo() const;
    int setup(USCData* p);
    int enumerateDevices();
    void activate();
    void doPlay(bool play);
    void deactivate();
    void destroy();
    // int getLastError();
    ~DataPortAudio();
};
