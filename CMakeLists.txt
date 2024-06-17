cmake_minimum_required(VERSION 3.0)

find_package(PkgConfig REQUIRED)

set(imgui_path ./submodules/imgui)
set(implot_path ./submodules/implot)
set(srcs_path ./src/)

set(cpl_opts "")
set(lnk_libs "")

# AUDIO LIBRARY
# definitely not stolen from somewhere
set(PA_BUILD_SHARED_LIBS OFF CACHE BOOL "Build dynamic library" FORCE)
add_subdirectory(submodules/portaudio EXCLUDE_FROM_ALL)
list(APPEND lnk_libs PortAudio)

# GUI SOURCES
set(gui_srcs
${imgui_path}/imgui.cpp
${imgui_path}/imgui_demo.cpp
${imgui_path}/imgui_draw.cpp
${imgui_path}/imgui_tables.cpp
${imgui_path}/imgui_widgets.cpp
${imgui_path}/misc/cpp/imgui_stdlib.cpp

${implot_path}/implot.cpp
${implot_path}/implot_demo.cpp
${implot_path}/implot_items.cpp)

list(APPEND gui_srcs
${imgui_path}/backends/imgui_impl_sdl2.cpp
${imgui_path}/backends/imgui_impl_opengl3.cpp)

add_subdirectory(submodules/SDL EXCLUDE_FROM_ALL)
list(APPEND lnk_libs SDL2)

pkg_check_modules(OPENGL opengl REQUIRED)
list(APPEND cpl_opts ${OPENGL_CFLAGS_OTHER})
list(APPEND lnk_libs ${OPENGL_LIBRARIES})
# SOURCES

set(unscope_srcs)

list(APPEND unscope_srcs ${aud_srcs} ${gui_srcs})
list(APPEND unscope_srcs 
${srcs_path}/audio/audioInput.cpp
${srcs_path}/audio/audioProcess.cpp

${srcs_path}/gui/gui.cpp
${srcs_path}/gui/consts.cpp
${srcs_path}/gui/wins/oscs.cpp
${srcs_path}/gui/wins/ctrls.cpp

${srcs_path}/main.cpp
)

project(unscope)
set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 1)

add_executable(unscope ${unscope_srcs})

target_include_directories(unscope SYSTEM PRIVATE
${SDL2_INCLUDE_DIRS}
${imgui_path}
${imgui_path}/misc/cpp
${imgui_path}/backends
${implot_path}
${srcs_path}
${srcs_path}/audio
${srcs_path}/gui)

target_compile_options(unscope PRIVATE ${cpl_opts})
target_link_libraries(unscope PRIVATE ${lnk_libs})

