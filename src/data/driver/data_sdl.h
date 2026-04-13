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
#include "SDL.h"
#include "SDL_audio.h"

class DataSDL : public DataDriver {
  SDL_AudioSpec request, response;
  vector<string> devices;
  int deviceNum, channels;
  int sampleRateNum, frameSizeNum;

  int deviceNumInternal;

  static void audioCallback(void* userdata, Uint8* stream, int len);

  public:
    const DataDriverInfo getDriverInfo() const;
    int setup(USCData* p);
    int enumerateDevices();
    void activate();
    void doPlay(bool play);
    void deactivate();
    void destroy();
    ~DataSDL();
};

