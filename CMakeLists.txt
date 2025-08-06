cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)

project(unscope)
set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 3)

set(linked_libraries)
set(compiler_options)

set(gui_sources)
set(audio_sources)
set(trigger_sources)

# paths
set(sources_path    ${CMAKE_SOURCE_DIR}/src/)
set(submodules_path ${CMAKE_SOURCE_DIR}/submodules)

set(imgui_path        ${submodules_path}/imgui)
set(imgui_toggle_path ${submodules_path}/imgui_toggle)
set(imgui_knobs_path  ${submodules_path}/imgui-knobs)
set(sdl_path          ${submodules_path}/SDL)

# target os detect
if(CMAKE_TOOLCHAIN_FILE)
  message(STATUS "toolcahin file set!")
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
  set(DIRECTX_DEF TRUE)
  set(OPENGL_DEF FALSE)
else()
  set(SYSTEM_SDL_DEF TRUE)
  set(DIRECTX_DEF FALSE)
  set(OPENGL_DEF TRUE)
endif()

option(SYSTEM_SDL "use system sdl" ${SYSTEM_SDL_DEF})
option(RENDER_DIRECTX "use directx" ${DIRECTX_DEF})
option(RENDER_OPENGL "use opengl" ${OPENGL_DEF})

# render
if (RENDER_OPENGL)
  message(STATUS "using opengl")
  pkg_check_modules(OPENGL opengl REQUIRED)
  list(APPEND compiler_options ${OPENGL_CFLAGS})
  list(APPEND linked_libraries ${OPENGL_LIBRARIES})
endif()

if(RENDER_DIRECTX)
  message(STATUS "using directx")
  list(APPEND linked_libraries "d3d11.lib")
endif()

# sdl
if (SYSTEM_SDL)
  pkg_check_modules(SDL sdl2 REQUIRED)
  list(APPEND compiler_options ${SDL_CFLAGS} ${SDL_CFLAGS_OTHER})
  list(APPEND linked_libraries ${SDL_LINK_LIBRARIES})
else()
  set(SDL_STATIC TRUE)
  add_subdirectory(${sdl_path} EXCLUDE_FROM_ALL)
  set(SDL_INCLUDE_DIRS ${sdl_path}/include)
  list(APPEND linked_libraries SDL2::SDL2main SDL2::SDL2)
  # list(APPEND linked_libraries SDL2)
  add_compile_definitions(NON_SYS_SDL)
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
set(SDL_OPENGL ${RENDER_OPENGL})
set(SDL_DIRECTX ${RENDER_DIRECTX})

# portaudio
set(PA_BUILD_SHARED_LIBS OFF CACHE BOOL "Build dynamic library" FORCE)
add_subdirectory(${submodules_path}/portaudio EXCLUDE_FROM_ALL)
list(APPEND linked_libraries PortAudio)

# sources
list(APPEND audio_sources
${sources_path}/audio/audio.cpp
${sources_path}/audio/io/abstract.cpp
${sources_path}/audio/io/audio_pa.cpp
)

list(APPEND trigger_sources
${sources_path}/trigger/params.cpp
${sources_path}/trigger/abstract.cpp
${sources_path}/trigger/fallback.cpp
${sources_path}/trigger/analog.cpp
${sources_path}/trigger/smooth.cpp
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

${sources_path}/gui/consts.cpp
${sources_path}/gui/gui.cpp
${sources_path}/gui/wins/about.cpp
${sources_path}/gui/wins/ctrls.cpp
${sources_path}/gui/wins/oscs.cpp
${sources_path}/gui/wins/cursors.cpp
${sources_path}/gui/wins/audioConfig.cpp

${sources_path}/gui/render/abstract.cpp
${sources_path}/gui/render/render_sdl2renderer.cpp)

if(RENDER_OPENGL)
  add_compile_definitions(USE_OPENGL)
  list(APPEND gui_sources
  ${imgui_path}/backends/imgui_impl_opengl2.cpp
  ${sources_path}/gui/render/render_opengl2.cpp)
endif()
if(RENDER_DIRECTX)
  add_compile_definitions(USE_DIRECTX)
  list(APPEND gui_sources
  ${imgui_path}/backends/imgui_impl_dx11.cpp
  ${sources_path}/gui/render/render_directx11.cpp)
  list(APPEND linked_libraries "d3dcompiler")
endif()

list(APPEND unscope_srcs ${gui_sources} ${audio_sources} ${trigger_sources} ${sources_path}/main.cpp)

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
${imgui_path}
${imgui_path}/misc/cpp
${imgui_path}/backends
${imgui_toggle_path}
${imgui_knobs_path}
${sources_path}
${sources_path}/audio
${sources_path}/audio/io
${sources_path}/trigger
${sources_path}/gui
${sources_path}/gui/render)

target_compile_options(unscope PRIVATE ${compiler_options})
target_link_libraries(unscope PRIVATE ${linked_libraries})

