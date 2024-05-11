cmake_minimum_required(VERSION 3.20)

find_package(PkgConfig REQUIRED)

set(pa_path ./submodules/portaudio/src)
set(imgui_path ./submodules/imgui)
set(implot_path ./submodules/implot)
set(srcs_path ./src/)

set(pa_includes
./submodules/portaudio/include/
./submodules/portaudio/src/common/
./submodules/portaudio/src/os/unix/)

set(cpl_opts "")
set(lnk_libs "")
# AUDIO SOURCES
set(aud_srcs
${pa_path}/common/pa_allocation.c
${pa_path}/common/pa_converters.c
${pa_path}/common/pa_cpuload.c
${pa_path}/common/pa_debugprint.c
${pa_path}/common/pa_dither.c
${pa_path}/common/pa_front.c
${pa_path}/common/pa_process.c
${pa_path}/common/pa_ringbuffer.c
${pa_path}/common/pa_stream.c
${pa_path}/common/pa_trace.c 

${pa_path}/os/unix/pa_pthread_util.c
${pa_path}/os/unix/pa_unix_hostapis.c
${pa_path}/os/unix/pa_unix_util.c)

option(ahost_alsa "build for alsa" ON)
option(ahost_pulse "build for pulseaudio" ON)
option(ahost_jack "build for jack" OFF)

if (ahost_alsa)
	list(APPEND aud_srcs ${pa_path}/hostapi/alsa/pa_linux_alsa.c)
	pkg_check_modules(ALSA alsa)
	list(APPEND cpl_opts ${ALSA_CFLAGS_OTHER})
	list(APPEND lnk_libs ${ALSA_LIBRARIES})
endif()

if (ahost_pulse)
	list(APPEND aud_srcs
		${pa_path}/hostapi/pulseaudio/pa_linux_pulseaudio_block.c
		${pa_path}/hostapi/pulseaudio/pa_linux_pulseaudio_cb.c
		${pa_path}/hostapi/pulseaudio/pa_linux_pulseaudio.c
		)
	pkg_check_modules(PULSE libpulse)
	list(APPEND cpl_opts ${PULSE_CFLAGS_OTHER})
	list(APPEND lnk_libs ${PULSE_LIBRARIES})
endif()

if (ahost_jack)
	list(APPEND aud_srcs ${pa_path}/hostapi/jack/pa_jack.c)
	pkg_check_modules(JACK jack2)
	list(APPEND cpl_opts ${JACK_CFLAGS_OTHER})
	list(APPEND lnk_libs ${JACK_LIBRARIES})
endif()

# GUI SOURCES
set(gui_srcs
${imgui_path}/imgui.cpp
${imgui_path}/imgui_demo.cpp
${imgui_path}/imgui_draw.cpp
${imgui_path}/imgui_tables.cpp
${imgui_path}/imgui_widgets.cpp

${implot_path}/implot.cpp
${implot_path}/implot_demo.cpp
${implot_path}/implot_items.cpp)

list(APPEND gui_srcs
${imgui_path}/backends/imgui_impl_sdl2.cpp
${imgui_path}/backends/imgui_impl_opengl3.cpp)

pkg_check_modules(SDL2 sdl2)
list(APPEND cpl_opts ${SDL2_CFLAGS_OTHER})
list(APPEND lnk_libs ${SDL2_LIBRARIES})

pkg_check_modules(OPENGL opengl)
list(APPEND cpl_opts ${OPENGL_CFLAGS_OTHER})
list(APPEND lnk_libs ${OPENGL_LIBRARIES})
# SOURCES

set(unscope_srcs)

list(APPEND unscope_srcs ${aud_srcs} ${gui_srcs})
list(APPEND unscope_srcs 
${srcs_path}/audio/audioInput.cpp
${srcs_path}/audio/audioProcess.cpp

${srcs_path}/gui/gui.cpp

${srcs_path}/main.cpp
)

project(unscope VERSION 0) #temporary name

add_executable(unscope ${unscope_srcs})

target_include_directories(unscope SYSTEM PRIVATE
${SDL2_INCLUDE_DIRS}
${pa_includes}
${imgui_path}
${imgui_path}/backends
${implot_path}
${srcs_path}
${srcs_path}/audio
${srcs_path}/gui)

target_compile_options(unscope PRIVATE "-lpthread" ${cpl_opts})
target_link_libraries(unscope PRIVATE ${lnk_libs})

