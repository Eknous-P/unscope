#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>

class GUI {
	private:
		bool running;
	public:
		bool isRunning();
		int init();
		// ~GUI();
};