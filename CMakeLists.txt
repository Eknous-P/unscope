cmake_minimum_required(VERSION 3.0)

if(WIN32) # windows shall use directx
  set(USE_OPENGL FALSE)
  set(USE_DIRECTX TRUE)
  add_compile_definitions(USE_DIRECTX)
else() # linux shall use opengl
  find_package(PkgConfig REQUIRED)
  set(USE_OPENGL TRUE)
  set(USE_DIRECTX FALSE)
  add_compile_definitions(USE_OPENGL)
endif()

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(imgui_path ./submodules/imgui)
set(implot_path ./submodules/implot)
set(srcs_path ./src/)

set(cpl_opts "")
set(lnk_libs "")

# AUDIO LIBRARY
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

if(USE_OPENGL)
  pkg_check_modules(OPENGL opengl REQUIRED)
  list(APPEND cpl_opts ${OPENGL_CFLAGS_OTHER})
  list(APPEND lnk_libs ${OPENGL_LIBRARIES})
  set(SDL_OPENGL ON)
else()
  set(SDL_OPENGL OFF)
endif()
if(USE_DIRECTX)
  set(SDL_DIRECTX ON)
  list(APPEND lnk_libs "d3d11.lib")
else()
  set(SDL_DIRECTX OFF)
endif()

set(SDL_AUDIO OFF)
set(SDL_HAPTIC OFF)
set(SDL_JOYSTICK OFF)
set(SDL_HIDAPI OFF)
set(SDL_POWER OFF)
set(SDL_FILE OFF)
set(SDL_FILESYSTEM OFF)
set(SDL_SENSOR OFF)
set(SDL_LOCALE OFF)

add_subdirectory(submodules/SDL EXCLUDE_FROM_ALL)
list(APPEND lnk_libs SDL2)


# SOURCES

set(unscope_srcs)

if(USE_OPENGL)
  list(APPEND unscope_srcs ${srcs_path}/gui/render/render_opengl_sdl.cpp)
  list(APPEND gui_srcs
  ${imgui_path}/backends/imgui_impl_sdl2.cpp
  ${imgui_path}/backends/imgui_impl_opengl3.cpp)
endif()
if(USE_DIRECTX)
  list(APPEND unscope_srcs ${srcs_path}/gui/render/render_directx11_sdl.cpp)
  list(APPEND gui_srcs
  ${imgui_path}/backends/imgui_impl_dx11.cpp
  ${imgui_path}/backends/imgui_impl_sdl2.cpp)
endif()

list(APPEND unscope_srcs ${aud_srcs} ${gui_srcs})
list(APPEND unscope_srcs 
${srcs_path}/audio/audioInput.cpp

${srcs_path}/gui/render/abstract.cpp
)

list(APPEND unscope_srcs 
${srcs_path}/gui/gui.cpp
${srcs_path}/gui/consts.cpp
${srcs_path}/gui/wins/oscs.cpp
${srcs_path}/gui/wins/ctrls.cpp

${srcs_path}/main.cpp
)

project(unscope)
set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 2)

if (NOT WIN32)
  add_executable(unscope ${unscope_srcs})
else()
  add_executable(unscope WIN32 ${unscope_srcs})
endif()

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

if(WIN32)
add_custom_command(TARGET unscope POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/submodules/SDL/SDL2d.dll"
        ${CMAKE_BINARY_DIR})
endif()