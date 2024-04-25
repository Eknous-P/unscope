cmake_minimum_required(VERSION 3.20)

set(pa_path submodules/portaudio)
set(imgui_path submodules/imgui)
set(srcs_path src/)
# AUDIO SOURCES
set(pa_srcs
${pa_path}/common/pa_allocation.c
${pa_path}/common/pa_converters.c
${pa_path}/common/pa_cpuload.c
${pa_path}/common/pa_debugprint.c
${pa_path}/common/pa_dither.c
${pa_path}/common/pa_front.c
${pa_path}/common/pa_process.c
${pa_path}/common/pa_ringbuffer.c
${pa_path}/common/pa_stream.c
${pa_path}/common/pa_trace.c )

option(ahost_alsa "build for alsa" ON)
option(ahost_jack "build for jack" OFF)

if (ahost_alsa)
	list(APPEND pa_srcs ${pa_path}/hostapi/alsa/pa_linux_alsa.c)
endif()

if (ahost_jack)
	list(APPEND pa_srcs ${pa_path}/hostapi/jack/pa_jack.c)
endif()

# GUI SOURCES
set(gui_srcs
${imgui_path}/imgui.cpp
${imgui_path}/imgui_demo.cpp
${imgui_path}/imgui_draw.cpp
${imgui_path}/imgui_tables.cpp
${imgui_path}/imgui_widgets.cpp)

list(APPEND gui_srcs
${imgui_path}/backends/imgui_impl_sdl2.cpp
${imgui_path}/backends/imgui_impl_opengl2.cpp)

# SOURCES

set(unscope_srcs
${srcs_path}/main.cpp
${srcs_path}/audioInput.cpp
${srcs_path}/audioProcess.cpp)

project(unscope VERSION 0) #temporary name




