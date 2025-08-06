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

#ifndef USC_SHARED_H
#define USC_SHARED_H

#include <vector>
using std::vector;
extern "C" {
#include <stdio.h>
#include <memory.h>
#include <math.h>
}

typedef float AudioSample;
typedef unsigned long long int nint;

// program stuff
#define PROGRAM_DEBUG

#define PROGRAM_NAME "unscope"
#define PROGRAM_VER "1.0rc1"

#define PROGRAM_NAME_AND_VER PROGRAM_NAME " " PROGRAM_VER

#define PROGRAM_WIDTH 1280
#define PROGRAM_HEIGHT 720

// printf fancies
#define ERROR_MSG "\033[31;1m"
#define INFO_MSG "\033[35;1m"
#define SUCCESS_MSG "\033[32;1m"
#define MISC_MSG "\033[34;1m"
#define MSG_END "\033[0m\n"

// frequently used stuff
#define FOR_RANGE(c) for (unsigned char z = 0; z < c; z++)
#define NEW_DOUBLE_PTR(x,type,size,count) \
  x = new type*[count]; \
  if (x) { \
    FOR_RANGE(count) { \
      x[z] = new type[size]; \
    } \
  }
#define DELETE_PTR(x) \
  if (x) { \
    delete x; \
    x = NULL; \
  }

#define DELETE_PTR_ARR(x) \
  if (x) { \
    delete[] x; \
    x = NULL; \
  }

#define DELETE_DOUBLE_PTR(x,n) \
  if (x) { \
    FOR_RANGE(n) { \
      DELETE_PTR(x[z]) \
    } \
    delete[] x; \
    x = NULL; \
  }

#define DELETE_DOUBLE_PTR_ARR(x,n) \
  if (x) { \
    FOR_RANGE(n) { \
      DELETE_PTR_ARR(x[z]) \
    } \
    delete[] x; \
    x = NULL; \
  }

// ui fancies
#define KNOBS_SIZE 50.0f

#define RIGHTCLICK_EXACT_INPUT(v,d,f) \
        if (ImGui::BeginPopupContextItem(#v "input")) { \
          if (ImGui::InputScalar("##" #v "input",d,v)) f; \
          ImGui::EndPopup(); \
        }

enum unscopeErrors {
  UGUIERROR_SETUPFAIL,
  UGUIERROR_INITFAIL
};

enum unscopeArgs : char {
  UPARAM_BUFFERSIZE   = 'b',
  UPARAM_FRAMESIZE    = 'f',
  UPARAM_CHANNELCOUNT = 'c',
  UPARAM_SAMPLERATE   = 's',
  UPARAM_ABOUT        = 'a',
  UPARAM_HELP         = 'h',
  UPARAM_LICENSE      = 'l',
  UPARAM_VERSION      = 'v'
};

enum USCRenderers {
  USC_RENDER_NONE=0,
  USC_RENDER_SDLRENDERER2,
  USC_RENDER_OPENGL2,
  USC_RENDER_DIRECTX11,
};

struct unscopeParams {
  nint audioBufferSize;

  // gui ...
  float timebase;
  float xyPersist;
  float scale;
  float trigger;

  int renderer;
};

struct AudioConfig {
  int inputDevice, outputDevice;
  int inputChannels, outputChannels;
  int sampleRate;
  int frameSize;
};

enum AudioDeviceDirections {
  DIR_IN  = 1<<0,
  DIR_OUT = 1<<1,
};

struct AudioDevice {
  unsigned char direction; // bit 0 - in, bit 1 - out
  int dev;
  int host;
  char devName[256];
  AudioDevice():
    direction(0),
    dev(0),
    host(0)
    {memset(devName, 0, 256);}
  AudioDevice(unsigned char di, int dv, int h, const char* dn) {
    direction = di;
    dev = dv;
    host = h;
    memcpy(devName, dn, 256);
  }
};

float clamp(float a);
extern const char *helpMsg, *verMsg, *aboutMsg, *thirdPartyMsg, *licenseMsg, *errMsgs[], *renderers[];
const char* getErrorMsg(int e);

#endif
