#include "audio/audio.h"
#include "gui/gui.h"
#include "../shared.h"

struct unscopeParams {
  unsigned int audioBufferSize;
  unsigned int audioFrameSize;
  unsigned char channels;
  unsigned int sampleRate;
  int audioDevice;

  // gui ...
  float timebase;
  float scale;
  float trigger;
  unscopeParams():
    audioBufferSize(65536), // N samples
    audioFrameSize(512),    // N samples
    channels(2),            // N (1-3)
    sampleRate(48000),      // N samples (only common values?)
    audioDevice(0),         // internal ID, gets overwritten anyway
    timebase(60),           // ms
    scale(1.0f),            // (no unit)
    trigger(0.0f) {}        // (no unit)
};

float clamp(float a) {
  if (a > 1.0f) return 1.0f;
  if (a < -1.0f) return -1.0f;
  return a;
}

std::string getErrorMsg(int e) {
  std::string errm = "cant init audio!\n";
  switch (e) {
    case UAUDIOERR_NOERR:
      return "";
    case UAUDIOERR_NODEVS:
      return errm + errMsgs[0];
    case UAUDIOERR_NODEV:
      return errm + errMsgs[1];
    case UAUDIOERR_NOSTART:
      return errm + errMsgs[2];
    case UAUDIOERR_NOGOOD:
      return errm + errMsgs[3];
    default: 
      return errm + Pa_GetErrorText(e) + '\n';
  }
}

int main(int argc, char** argv) {
  unscopeParams params;
  parseParams(params, argc, argv);

  USCGUI g(params.sampleRate,
        params.audioBufferSize,
        params.channels,
        params.timebase,
        params.scale,
        params.trigger);

  g.init();

   USCAudioInput i(params.audioFrameSize,
               params.audioBufferSize,
               params.channels,
               params.sampleRate);

  int e;
  g.attachAudioInput(&i);

  params.audioDevice = Pa_GetDefaultInputDevice();

  g.getDevices(i.enumerateDevs());
  g.setAudioDeviceSetting(params.audioDevice);

  printf("opening device %d: %s ...\n",params.audioDevice,Pa_GetDeviceInfo(params.audioDevice)->name);
  e = i.init(params.audioDevice,0);
  if (e != paNoError) {
    printf("%d:%s", e, getErrorMsg(e).c_str());
    // try again
    if (e != paInvalidDevice) {
      printf("%d:%s", e, getErrorMsg(e).c_str());
      return e;
    }
    printf("trying default device...\n");
    params.audioDevice = Pa_GetDefaultInputDevice();
    e = i.init(params.audioDevice,0);
    if (e != paNoError) {
      printf("%d:%s", e, getErrorMsg(e).c_str());
      return e;
    }
    g.setAudioDeviceSetting(params.audioDevice);
  }
  while (g.isRunning()) {
    g.doFrame();
  }
  e = i.stop();
  printf("%s\n", Pa_GetErrorText(e));
  return e!=paNoError;
}

const char* verMsg = PROGRAM_NAME " (version " PROGRAM_VER ")\n";
const char* helpMsg =
"Program arguments\n"
"  -h: print this message\n"
"  -b: set the audio buffer size (default: 65536)\n"
"  -f: set the audio frame size (default: 512)\n"
"  -c: set the channel amount (default: 2)\n"
"  -s: set the sample rate (default: 48000)\n"
"  -v: print the program version\n";
const char* errMsgs[] = {
  "no devices found\n",
  "cant open device\n",
  "cant start device\n",
  "cant init portaudio\n"
};