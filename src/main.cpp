#include "audio/audio.h"
#include "gui/gui.h"
#include <iostream>

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
  if (e!=NOERR) {
    std::cout << "cant init audio!\n";
    switch (e) {
      case NODEVS:
        std::cout << "no devices found\n";
        return 1;
      case NODEV:
        std::cout << "cant open device\n";
        return 1;
      case NOSTART:
        std::cout << "cant start device\n";
        return 1;
      case NOGOOD:
        std::cout << "cant init portaudio\n";
        return 1;
      default:
        std::cout << Pa_GetErrorText(e) << '\n';
        return 1;
    }
  }

  AudioProcess p(DEF.bufferSize*DEF.channels);

  while (g.isRunning()) {
    p.writeDataIn(i.getData());
    g.writeOscData(
      p.alignWave(g.getTrigger(),g.getTraceSize(),0,0),
      i.getData());
    g.doFrame();
  }
  e = i.stop();
  std::cout << Pa_GetErrorText(e) << '\n';
  return e=!paNoError;
}