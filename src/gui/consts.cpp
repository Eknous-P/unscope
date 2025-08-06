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

#include "gui.h"

const char *windowLayout = "\
[Window][DockSpaceViewport_11111111]\n\
Pos=0,19\n\
Size=1280,701\n\
Collapsed=0\n\
\n\
[Window][Debug##Default]\n\
Pos=60,60\n\
Size=400,400\n\
Collapsed=0\n\
\n\
[Window][Controls]\n\
Pos=725,0\n\
Size=275,167\n\
Collapsed=0\n\
DockId=0x00000002,0\n\
\n\
[Window][Scope]\n\
Pos=0,19\n\
Size=978,479\n\
Collapsed=0\n\
DockId=0x00000003,0\n\
\n\
[Window][Scope (XY)]\n\
Pos=980,19\n\
Size=300,300\n\
Collapsed=0\n\
DockId=0x0000000B,0\n\
\n\
[Window][Global Controls]\n\
Pos=980,500\n\
Size=300,220\n\
Collapsed=0\n\
DockId=0x0000000E,1\n\
\n\
[Window][Cursors]\n\
Pos=980,500\n\
Size=300,220\n\
Collapsed=0\n\
DockId=0x0000000E,0\n\
\n\
[Window][Channel 1 Controls]\n\
Pos=0,500\n\
Size=488,220\n\
Collapsed=0\n\
DockId=0x00000005,0\n\
\n\
[Window][Channel 2 Controls]\n\
Pos=490,500\n\
Size=488,220\n\
Collapsed=0\n\
DockId=0x00000006,0\n\
\n\
[Window][XY Scope Controls]\n\
Pos=980,323\n\
Size=300,166\n\
Collapsed=0\n\
DockId=0x0000000D,0\n\
\n\
[Window][WindowOverViewport_11111111]\n\
Pos=0,19\n\
Size=1280,701\n\
Collapsed=0\n\
\n\
[Docking][Data]\n\
DockSpace       ID=0x8B93E3BD Pos=0,19 Size=1280,701 Split=X Selected=0x7C3EDFF1\n\
  DockNode      ID=0x00000001 Parent=0x8B93E3BD SizeRef=578,371 CentralNode=1 Selected=0x7C3EDFF1\n\
  DockNode      ID=0x00000002 Parent=0x8B93E3BD SizeRef=275,371 HiddenTabBar=1 Selected=0x67284010\n\
DockSpace       ID=0xD6C4A87D Window=0x1BBC0F80 Pos=0,19 Size=1280,701 Split=X Selected=0xB159AA79\n\
  DockNode      ID=0x00000009 Parent=0xD6C4A87D SizeRef=978,701 Split=Y\n\
    DockNode    ID=0x00000003 Parent=0x00000009 SizeRef=1280,470 CentralNode=1 Selected=0xB159AA79\n\
    DockNode    ID=0x00000004 Parent=0x00000009 SizeRef=1280,229 Split=X Selected=0xDA8B5EC4\n\
      DockNode  ID=0x00000005 Parent=0x00000004 SizeRef=508,220 Selected=0x2682A10E\n\
      DockNode  ID=0x00000006 Parent=0x00000004 SizeRef=468,220 Selected=0xDA8B5EC4\n\
  DockNode      ID=0x0000000A Parent=0xD6C4A87D SizeRef=300,701 Split=Y Selected=0x824BF046\n\
    DockNode    ID=0x0000000B Parent=0x0000000A SizeRef=300,302 Selected=0xA488D3FA\n\
    DockNode    ID=0x0000000C Parent=0x0000000A SizeRef=300,397 Split=Y Selected=0x824BF046\n\
      DockNode  ID=0x0000000D Parent=0x0000000C SizeRef=300,166 Selected=0x824BF046\n\
      DockNode  ID=0x0000000E Parent=0x0000000C SizeRef=300,229 Selected=0x15C72E9A\n\
";

const char* triggerNames[]={
  "fallback",
  "analog",
  "smoothed"
};

const unsigned char step_one = 1;

const int sampleRates[9]={
  8000,
  11025,
  16000,
  22050,
  32000,
  44100,
  48000,
  96000,
  192000
};

const int frameSizes[8]={
  16, 32, 64, 128,
  256, 512, 1024, 2048
};
