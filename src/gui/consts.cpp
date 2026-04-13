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

#include "consts.h"

const char *windowLayout = "\
";

const char* const dataDriverNames[]={
  "dummy",
  "PortAudio",
  "SDL",
#ifdef USE_PIPEWIRE_DEFINE
  "PipeWire"
#endif
};

const char* const triggerNames[]={
  "fallback",
  "analog",
  "smoothed"
};

const char* const scopeModes[]={
  "Trace",
  "XY",
  "Spectrum"
};

const char* const spectrumModes[]={
  "FFT",
  "Constant Q"
};

const unsigned char step_one = 1;
