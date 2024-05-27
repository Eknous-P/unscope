# unscope

Audio oscilloscope for Linux

## Reporting issues

Please do! Report them in the *Issues* tab.

## Building

(No release yet! But I'm getting closer...)

### Getting the source

Open a terminal and enter these commands to clone this repository (make sure you have git installed!).
```
git clone https://github.com/Eknous-P/unscope.git --recursive
cd unscope
```

- If you forget to clone recursively, you will have to enter
`git submodule update --init --recursive`
to get the submodules.

### Getting the libraries

Currently unscope requires the OpenGL, ALSA and PulseAudio development libraries to be installed

#### Ubuntu and Ubuntu-based
```
sudo apt install libasound2-dev libpulse-dev libopengl-dev libxext-dev
```

### Building

```
mkdir build
cd build
cmake ..
make
```