/*
unscope - an audio oscilloscope
Copyright (C) 2025 Eknous

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

#include "audio.h"
#include "portaudio.h"

class Audio_PA : public AudioIO {
  PaStream* stream;
  PaError e;
  PaStreamParameters streamParamsI, streamParamsO;

  USCAudio* parent;
  AudioConfig* conf;
  AudioBuffer* buf;
  vector<AudioDevice>* devs;
  int (*callbackFunction)(void*, const void*, void*, nint);
  bool running, outputting;

  void openStream(int iDev, int oDev, int iChans, int oChans, int sampleRate, int frames);
  void closeStream();

  static int audioCallback(
    const void *inputBuffer, void *outputBuffer,
    unsigned long int framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData);

  public:
    int setup(USCAudio* p);
    int init();
    int deinit();
    int start();
    int stop();
    bool isRunning();
    bool isOutputting();
    int getAvailDevices();
    int getDefaultInputDevice();
    int getDefaultOutputDevice();
    const char* getLastError();
};
