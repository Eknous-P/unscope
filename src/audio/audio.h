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

#ifndef USC_AUDIO_H
#define USC_AUDIO_H

#include "shared.h"

struct AudioBuffer {
  int chans;
  nint size;
  AudioSample **data;
  AudioBuffer():
    chans(0),
    size(0),
    data(NULL) {}
};

enum AudioDrivers {
  AUDIO_DUMMY=0,
  AUDIO_PORTAUDIO,
};

class AudioIO;

class USCAudio {
  private:
    AudioConfig* config;
    AudioBuffer buffer;

    AudioIO* audioIO;

    vector<AudioDevice> devices;

    float loopbackVolume;

    static int internalAudioCallback(
      void* self,
      const void* audioInput,
      void* audioOutput,
      nint frames);
  public:
    AudioConfig* getConfig();
    AudioBuffer* getBuffer();
    vector<AudioDevice>* getDevices();
    int(*getCallbackFunction())(void*, const void*, void*, nint);

    int initIO(AudioDrivers d);

    void setLoopback(float v);
    int getDefaultInputDevice();
    int getDefaultOutputDevice();
    const char* getLastIOError();
    int initAudio();
    int startAudio();
    int stopAudio();
    int deinitAudio();

    int deinitIO();

    int getAvailDevices();
    AudioSample** getAudioBuffer();

    bool isOutputting();

    USCAudio(AudioConfig* conf, nint bufSize);
    ~USCAudio();
};

class AudioIO {
  USCAudio* parent;
  AudioConfig* conf;
  AudioBuffer* buf;
  vector<AudioDevice>* devs;
  int (*callbackFunction)(void*, const void*, void*, nint);
  bool running, outputting;
  public:
    virtual int setup(USCAudio* p);
    virtual int init();
    virtual int deinit();
    virtual int start();
    virtual int stop();
    virtual bool isRunning();
    virtual bool isOutputting();
    virtual int getAvailDevices();
    virtual int getDefaultInputDevice();
    virtual int getDefaultOutputDevice();
    virtual const char* getLastError();
};

#endif
