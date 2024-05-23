#include "audio/audio.h"
#include "gui/gui.h"
#include <string>

struct unscopeDefaults {
  unsigned int audioBufferSize;
  unsigned int audioFrameSize;
  unsigned char channels;
  unsigned int sampleRate;

  // gui ...
  unsigned int bufferSize;
  float scale;
  float trigger;
  unscopeDefaults():
    audioBufferSize(65536),
    audioFrameSize(128),
    channels(1),
    sampleRate(48000),
    bufferSize(1600),
    scale(2.0f),
    trigger(0.0f) {}
};

const char* errMsgs[] = {
  "no devices found\n",
  "cant open device\n",
  "cant start device\n",
  "cant init portaudio\n"
};

std::string getErrorMsg(int e) {
  std::string errm = "cant init audio!\n";
  switch (e) {
    case NOERR:
      return "";
    case NODEVS:
      return errm + errMsgs[0];
    case NODEV:
      return errm + errMsgs[1];
    case NOSTART:
      return errm + errMsgs[2];
    case NOGOOD:
      return errm + errMsgs[3];
    default: 
      return errm + Pa_GetErrorText(e) + '\n';
  }
}

int main() {
  unscopeDefaults DEF;

  GUI g(DEF.audioBufferSize,
        DEF.channels,
        DEF.bufferSize,
        DEF.scale,
        DEF.trigger);

  g.init();

  AudioInput i(DEF.audioFrameSize,
               DEF.audioBufferSize,
               DEF.channels,
               DEF.sampleRate);

  int e;

  e = i.init(Pa_GetDefaultInputDevice());
  if (e =! paNoError) printf("%s", getErrorMsg(e).c_str());

  AudioProcess p(DEF.bufferSize*DEF.channels);

  while (g.isRunning()) {
    p.writeDataIn(i.getData());
    g.writeOscData(
      p.alignWave(g.getTrigger(),g.getTraceSize(),0,0),
      i.getData());
    g.doFrame();
  }
  e = i.stop();
  printf("%s\n", Pa_GetErrorText(e));
  return e=!paNoError;
}