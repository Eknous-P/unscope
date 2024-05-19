#include "audio/audio.h"
#include "gui/gui.h"
#include <iostream>

int main() {
  AudioInput i(1,48000);
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

  AudioProcess p(i.getDataSize()*i.getChanCount());
  GUI g(i.getDataSize(), i.getChanCount(), 1600, 2.0f, 0.0f);
  g.init();
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