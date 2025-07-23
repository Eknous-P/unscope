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
DockId=0x00000009,0\n\
\n\
[Window][Scope (XY)]\n\
Pos=980,19\n\
Size=300,316\n\
Collapsed=0\n\
DockId=0x0000000B,0\n\
\n\
[Window][Global Controls]\n\
Pos=980,500\n\
Size=300,220\n\
Collapsed=0\n\
DockId=0x00000004,0\n\
\n\
[Window][Cursors]\n\
Pos=980,500\n\
Size=300,220\n\
Collapsed=0\n\
DockId=0x00000004,1\n\
\n\
[Window][Channel 1 Controls]\n\
Pos=0,500\n\
Size=488,220\n\
Collapsed=0\n\
DockId=0x00000007,0\n\
\n\
[Window][Channel 2 Controls]\n\
Pos=490,500\n\
Size=488,220\n\
Collapsed=0\n\
DockId=0x00000003,0\n\
\n\
[Window][XY Scope Controls]\n\
Pos=980,337\n\
Size=300,161\n\
Collapsed=0\n\
DockId=0x0000000C,0\n\
\n\
[Docking][Data]\n\
DockSpace         ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,19 Size=1280,701 Split=X Selected=0x7C3EDFF1\n\
  DockNode        ID=0x00000001 Parent=0x8B93E3BD SizeRef=578,371 Split=Y Selected=0x7C3EDFF1\n\
    DockNode      ID=0x00000005 Parent=0x00000001 SizeRef=1003,498 Split=X Selected=0x7C3EDFF1\n\
      DockNode    ID=0x00000009 Parent=0x00000005 SizeRef=978,511 CentralNode=1 Selected=0x7C3EDFF1\n\
      DockNode    ID=0x0000000A Parent=0x00000005 SizeRef=300,511 Split=Y Selected=0x5D48DF31\n\
        DockNode  ID=0x0000000B Parent=0x0000000A SizeRef=360,316 Selected=0x5D48DF31\n\
        DockNode  ID=0x0000000C Parent=0x0000000A SizeRef=360,161 Selected=0x9828E27C\n\
    DockNode      ID=0x00000006 Parent=0x00000001 SizeRef=1003,220 Split=X Selected=0x5E07700B\n\
      DockNode    ID=0x00000007 Parent=0x00000006 SizeRef=488,207 Selected=0x5E07700B\n\
      DockNode    ID=0x00000008 Parent=0x00000006 SizeRef=790,207 Split=X Selected=0x226655D0\n\
        DockNode  ID=0x00000003 Parent=0x00000008 SizeRef=488,207 Selected=0x226655D0\n\
        DockNode  ID=0x00000004 Parent=0x00000008 SizeRef=300,207 Selected=0x226655D0\n\
  DockNode        ID=0x00000002 Parent=0x8B93E3BD SizeRef=275,371 HiddenTabBar=1 Selected=0x67284010\n\
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
