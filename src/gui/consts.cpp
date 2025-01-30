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
Size=980,480\n\
Collapsed=0\n\
DockId=0x00000009,0\n\
\n\
[Window][Scope (XY)]\n\
Pos=981,19\n\
Size=300,300\n\
Collapsed=0\n\
DockId=0x0000000B,0\n\
\n\
[Window][Global Controls]\n\
Pos=981,500\n\
Size=299,220\n\
Collapsed=0\n\
DockId=0x00000004,0\n\
\n\
[Window][Channel 1 Controls]\n\
Pos=0,500\n\
Size=460,220\n\
Collapsed=0\n\
DockId=0x00000007,0\n\
\n\
[Window][Channel 2 Controls]\n\
Pos=460,500\n\
Size=460,220\n\
Collapsed=0\n\
DockId=0x00000003,0\n\
\n\
[Window][XY Scope Controls]\n\
Pos=981,329\n\
Size=299,169\n\
Collapsed=0\n\
DockId=0x0000000C,0\n\
\n\
[Docking][Data]\n\
DockSpace         ID=0x8B93E3BD Window=0xA787BDB4 Pos=0,19 Size=1280,701 Split=X Selected=0x7C3EDFF1\n\
  DockNode        ID=0x00000001 Parent=0x8B93E3BD SizeRef=578,371 Split=Y Selected=0x7C3EDFF1\n\
    DockNode      ID=0x00000005 Parent=0x00000001 SizeRef=1003,498 Split=X Selected=0x7C3EDFF1\n\
      DockNode    ID=0x00000009 Parent=0x00000005 SizeRef=979,511 CentralNode=1 Selected=0x7C3EDFF1\n\
      DockNode    ID=0x0000000A Parent=0x00000005 SizeRef=299,511 Split=Y Selected=0x5D48DF31\n\
        DockNode  ID=0x0000000B Parent=0x0000000A SizeRef=360,308 Selected=0x5D48DF31\n\
        DockNode  ID=0x0000000C Parent=0x0000000A SizeRef=360,169 Selected=0x9828E27C\n\
    DockNode      ID=0x00000006 Parent=0x00000001 SizeRef=1003,220 Split=X Selected=0x5E07700B\n\
      DockNode    ID=0x00000007 Parent=0x00000006 SizeRef=481,207 Selected=0x5E07700B\n\
      DockNode    ID=0x00000008 Parent=0x00000006 SizeRef=797,207 Split=X Selected=0x226655D0\n\
        DockNode  ID=0x00000003 Parent=0x00000008 SizeRef=497,207 Selected=0x226655D0\n\
        DockNode  ID=0x00000004 Parent=0x00000008 SizeRef=299,207 Selected=0xDFE49EEC\n\
  DockNode        ID=0x00000002 Parent=0x8B93E3BD SizeRef=275,371 HiddenTabBar=1 Selected=0x67284010\n\
";

const char* triggerModeNames[4]={
  "none",
  "auto",
  "normal",
  "single"
};

const char* triggerNames[]={
  "fallback",
  "analog"
};
