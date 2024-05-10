#include "audio/audio.h"
#include "gui/gui.h"
#include <iostream>

int main() {
	AudioInput i;
	GUI g;
	g.init();
	i.~AudioInput();
	while (g.isRunning()) {
		g.writeOscData(i.getData(),i.getDataSize());
		g.doFrame();
	}
	return 0;
}