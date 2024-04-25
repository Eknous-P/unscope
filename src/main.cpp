#include "audio/audio.h"
#include "gui/gui.h"
#include <iostream>

int main() {
	AudioInput i;
	GUI g;
	g.init();
	i.~AudioInput();
	return 0;
}