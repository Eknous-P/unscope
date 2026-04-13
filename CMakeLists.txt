cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 11)

project(unscope)
set(CMAKE_PROJECT_VERSION_MAJOR 1)
set(CMAKE_PROJECT_VERSION_MINOR 0)

set(linked_libraries)
set(compiler_options)

set(config_sources)
set(gui_sources)
set(audio_sources)
set(trigger_sources)
set(param_sources)

# paths
set(sources_path    ${CMAKE_SOURCE_DIR}/src/)
set(submodules_path ${CMAKE_SOURCE_DIR}/submodules)

set(imgui_path        ${submodules_path}/imgui)
set(imgui_toggle_path ${submodules_path}/imgui_toggle)
set(imgui_knobs_path  ${submodules_path}/imgui-knobs)
set(sdl_path          ${submodules_path}/SDL)

# target os detect
if(CMAKE_TOOLCHAIN_FILE)
  message(STATUS "toolchain file set!")
endif()

if (WIN32)
  set(IS_WIN TRUE)
  message(STATUS "building for windows")
  if (MINGW)
    message(STATUS "using mingw")
  endif()
else()
  set(IS_WIN FALSE)
  message(STATUS "building for linux")
  find_package(PkgConfig REQUIRED)
endif()

# flags
if (IS_WIN)
  set(SYSTEM_SDL_DEF FALSE)
  set(USE_PIPEWIRE_DEF FALSE)
  if (IS_32BIT)
    set(DIRECTX9_DEF TRUE)
    set(DIRECTX11_DEF FALSE)
  else()
    set(DIRECTX9_DEF FALSE)
    set(DIRECTX11_DEF TRUE)
  endif()

  set(OPENGL_DEF FALSE)
else()
  set(USE_PIPEWIRE_DEF TRUE)
  set(SYSTEM_SDL_DEF TRUE)
  set(DIRECTX9_DEF FALSE)
  set(DIRECTX11_DEF FALSE)
  set(OPENGL_DEF TRUE)
endif()

option(SYSTEM_SDL "use system sdl" ${SYSTEM_SDL_DEF})
option(RENDER_DIRECTX9 "use directx9" ${DIRECTX9_DEF})
option(RENDER_DIRECTX11 "use directx11" ${DIRECTX11_DEF})
option(RENDER_OPENGL "use opengl" ${OPENGL_DEF})
option(USE_PIPEWIRE "build with pipewire support" ${USE_PIPEWIRE_DEF})

# render
if (RENDER_OPENGL)
  message(STATUS "using opengl")
  pkg_check_modules(OPENGL opengl REQUIRED)
  list(APPEND compiler_options ${OPENGL_CFLAGS})
  list(APPEND linked_libraries ${OPENGL_LIBRARIES})
endif()

if(RENDER_DIRECTX9)
  message(STATUS "using directx9")
  list(APPEND linked_libraries "d3d9.lib")
endif()

if(RENDER_DIRECTX11)
  message(STATUS "using directx11")
  list(APPEND linked_libraries "d3d11.lib")
endif()

set(SDL_AUDIO ON CACHE BOOL "use sdl audio")
set(SDL_HAPTIC OFF CACHE BOOL "use sdl haptic")
set(SDL_JOYSTICK OFF CACHE BOOL "use sdl joystick")
set(SDL_HIDAPI OFF CACHE BOOL "use sdl hidapi")
set(SDL_POWER OFF CACHE BOOL "use sdl power")
set(SDL_FILE OFF CACHE BOOL "use sdl file")
set(SDL_FILESYSTEM ON CACHE BOOL "use sdl filesystem")
set(SDL_SENSOR OFF CACHE BOOL "use sdl sensor")
set(SDL_LOCALE OFF CACHE BOOL "use sdl locale")
set(SDL_OPENGL ${RENDER_OPENGL})
set(SDL_DIRECTX ${RENDER_DIRECTX11} OR ${RENDER_DIRECTX9})
set(SDL_PIPEWIRE OFF CACHE BOOL "use sdl pipewire")

# pffft
# it shouldnt be this high up but it lacks custom target checks so this is a workaround
set(PFFFT_USE_TYPE_DOUBLE OFF)
add_subdirectory(${submodules_path}/pffft EXCLUDE_FROM_ALL)
list(APPEND linked_libraries PFFFT)

# sdl
if (SYSTEM_SDL)
  pkg_check_modules(SDL sdl2 REQUIRED)
  list(APPEND compiler_options ${SDL_CFLAGS} ${SDL_CFLAGS_OTHER})
  list(APPEND linked_libraries ${SDL_LINK_LIBRARIES})
else()
  set(SDL_SHARED OFF CACHE BOOL "uhh")
  set(SDL_STATIC ON CACHE BOOL "statically link sdl")
  set(SDL_LIBC ON CACHE BOOL "use own libc for sdl")
  add_subdirectory(${sdl_path} EXCLUDE_FROM_ALL)
  set(SDL_INCLUDE_DIRS ${sdl_path}/include)
  # list(APPEND linked_libraries SDL2::SDL2)
  list(APPEND linked_libraries SDL2-static)
  add_compile_definitions(NON_SYS_SDL)
endif()

# portaudio
set(PA_BUILD_SHARED_LIBS OFF CACHE BOOL "Build dynamic library" FORCE)
add_subdirectory(${submodules_path}/portaudio EXCLUDE_FROM_ALL)
list(APPEND linked_libraries PortAudio)

# yaml-cpp
add_subdirectory(${submodules_path}/yaml-cpp EXCLUDE_FROM_ALL)
list(APPEND linked_libraries yaml-cpp)

# pipewire
if (USE_PIPEWIRE)
  pkg_check_modules(PIPEWIRE libpipewire-0.3)
  if(PIPEWIRE_FOUND)
  add_compile_definitions(USE_PIPEWIRE_DEFINE)
    list(APPEND compiler_options ${PIPEWIRE_CFLAGS} ${PIPEWIRE_CFLAGS_OTHER})
    list(APPEND linked_libraries ${PIPEWIRE_LIBRARIES})
  endif()
endif()

# sources
list(APPEND config_sources
${sources_path}/config/config.cpp
)

list(APPEND data_sources
${sources_path}/data/data.cpp
${sources_path}/data/buffer.cpp
${sources_path}/data/driver/abstract.cpp
# ${sources_path}/data/driver/dummy.cpp
${sources_path}/data/driver/audio_common.cpp
${sources_path}/data/driver/data_portaudio.cpp
${sources_path}/data/driver/data_sdl.cpp
)

if(USE_PIPEWIRE)
  list(APPEND data_sources ${sources_path}/data/driver/data_pipewire.cpp)
endif()

list(APPEND trigger_sources
${sources_path}/trigger/abstract.cpp
${sources_path}/trigger/fallback.cpp
${sources_path}/trigger/analog.cpp
${sources_path}/trigger/smooth.cpp
)

list(APPEND param_sources
${sources_path}/param/param.cpp
)

list(APPEND gui_sources
${imgui_path}/backends/imgui_impl_sdlrenderer2.cpp
${imgui_path}/backends/imgui_impl_sdl2.cpp
${imgui_path}/imgui.cpp
${imgui_path}/imgui_demo.cpp
${imgui_path}/imgui_draw.cpp
${imgui_path}/imgui_tables.cpp
${imgui_path}/imgui_widgets.cpp
${imgui_path}/misc/cpp/imgui_stdlib.cpp

${imgui_knobs_path}/imgui-knobs.cpp

${imgui_toggle_path}/imgui_toggle_palette.cpp
${imgui_toggle_path}/imgui_toggle_presets.cpp
${imgui_toggle_path}/imgui_toggle_renderer.cpp
${imgui_toggle_path}/imgui_toggle.cpp

${submodules_path}/showcqt-min/showcqt.c

${sources_path}/gui/font_fontaudio.cpp
${sources_path}/gui/consts.cpp

${sources_path}/gui/gui.cpp
${sources_path}/gui/scope/scope.cpp
${sources_path}/gui/wins/about.cpp
${sources_path}/gui/wins/ctrls.cpp
${sources_path}/gui/wins/settings.cpp
${sources_path}/gui/wins/dataConfig.cpp
# ${sources_path}/gui/wins/spectrum.cpp

${sources_path}/gui/render/abstract.cpp
${sources_path}/gui/render/render_sdl2renderer.cpp)

if(RENDER_OPENGL)
  add_compile_definitions(USE_OPENGL)
  list(APPEND gui_sources
  ${imgui_path}/backends/imgui_impl_opengl2.cpp
  ${sources_path}/gui/render/render_opengl2.cpp)
endif()
if(RENDER_DIRECTX9)
  add_compile_definitions(USE_DIRECTX9)
  list(APPEND gui_sources
  ${imgui_path}/backends/imgui_impl_dx9.cpp
  ${sources_path}/gui/render/render_directx9.cpp)
  list(APPEND linked_libraries "d3dcompiler")
endif()
if(RENDER_DIRECTX11)
  add_compile_definitions(USE_DIRECTX11)
  list(APPEND gui_sources
  ${imgui_path}/backends/imgui_impl_dx11.cpp
  ${sources_path}/gui/render/render_directx11.cpp)
  list(APPEND linked_libraries "d3dcompiler")
endif()
if(IS_32BIT)
  add_compile_definitions(BUILD_32BIT)
endif()

list(APPEND unscope_srcs ${config_sources} ${gui_sources} ${data_sources} ${trigger_sources} ${param_sources} ${sources_path}/main.cpp)

# project

add_compile_definitions(IMGUI_USER_CONFIG="gui/user_config.h")

if (IS_WIN)
  add_executable(unscope WIN32 ${unscope_srcs})
else()
  add_executable(unscope ${unscope_srcs})
endif()

if (MINGW)
  set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
endif()

target_include_directories(unscope SYSTEM PRIVATE
${SDL_INCLUDE_DIRS}
${PIPEWIRE_INCLUDE_DIRS}
${imgui_path}
${imgui_path}/misc/cpp
${imgui_path}/backends
${imgui_toggle_path}
${imgui_knobs_path}
${submodules_path}/showcqt-min
${sources_path}
${sources_path}/config
${sources_path}/param
${sources_path}/data
${sources_path}/data/driver
${sources_path}/trigger
${sources_path}/gui
${sources_path}/gui/scope
${sources_path}/gui/render)

target_compile_options(unscope PRIVATE ${compiler_options})
target_link_libraries(unscope PRIVATE ${linked_libraries})
