#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <SDL.h>
#include <SDL_opengl.h>

#define PROGRAM_NAME "unscope"
#define PROGRAM_VER 0

class GUI {
	private:
		SDL_WindowFlags window_flags;
		SDL_Window* window;
		SDL_GLContext gl_context;
		ImGuiIO io;
		ImGuiStyle style;
		ImVec4 clear_color;
		bool running;
		float* oscData;
		unsigned int oscDataSize;
	public:
		void writeOscData(void* data, unsigned int size);
		bool isRunning();
		int init();
		void doFrame();
		void drawGUI();
		GUI();
		~GUI();
};