cmake_minimum_required(VERSION 3.25)
project(RFFS_Online_V5 LANGUAGES C CXX)

# Recovered V5 sources. This is a reconstruction of the missing player/CMakeLists.txt.
# It has not yet been proven to reproduce the V6 binary or its save-state format.
if(NOT WIN32)
  message(FATAL_ERROR "This player was written for Windows.")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RFFS_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(SDL_ROOT "${RFFS_ROOT}/SDL3/SDL3-3.4.18")
set(IMGUI_ROOT "${RFFS_ROOT}/imgui")
set(GGPO_ROOT "${RFFS_ROOT}/ggpo")
set(FBNEO_ROOT "${RFFS_ROOT}/FBNeo")

foreach(required
  "${SDL_ROOT}/include/SDL3/SDL.h"
  "${SDL_ROOT}/lib/x64/SDL3.lib"
  "${IMGUI_ROOT}/imgui.cpp"
  "${GGPO_ROOT}/CMakeLists.txt"
  "${FBNEO_ROOT}/src/burner/libretro/libretro-common/include/libretro.h")
  if(NOT EXISTS "${required}")
    message(FATAL_ERROR "Missing required dependency: ${required}. Read recovered-v5/README.md.")
  endif()
endforeach()

set(GGPO_BUILD_VECTORWAR OFF CACHE BOOL "" FORCE)
set(GGPO_BUILD_SDK ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory("${GGPO_ROOT}" "${CMAKE_BINARY_DIR}/ggpo-build")
# GGPO upstream adds MSVC-only /MP and /GL; clang-cl reports those as unused.
# Suppress this compiler-option diagnostic only for the recovered GGPO target.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(GGPO PRIVATE -Wno-unused-command-line-argument -Wno-format -Wno-extern-c-compat -Wno-overloaded-virtual -Wno-reorder-ctor -Wno-missing-field-initializers -Wno-missing-braces -Wno-switch)
endif()

add_library(imgui_rffs STATIC
  "${IMGUI_ROOT}/imgui.cpp"
  "${IMGUI_ROOT}/imgui_draw.cpp"
  "${IMGUI_ROOT}/imgui_tables.cpp"
  "${IMGUI_ROOT}/imgui_widgets.cpp"
  "${IMGUI_ROOT}/backends/imgui_impl_sdl3.cpp"
  "${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp"
  "${IMGUI_ROOT}/backends/imgui_impl_sdlrenderer3.cpp")
target_include_directories(imgui_rffs PUBLIC "${IMGUI_ROOT}" "${SDL_ROOT}/include")
target_link_libraries(imgui_rffs PUBLIC "${SDL_ROOT}/lib/x64/SDL3.lib")

add_executable(RFFS-Online-v5 WIN32
  "${RFFS_ROOT}/rffs_player.cpp"
  "${RFFS_ROOT}/rffs_net.cpp"
  "${RFFS_ROOT}/rffs_steam.cpp"
  "${RFFS_ROOT}/rffs_menu.cpp"
  "${RFFS_ROOT}/rffs_gl.cpp"
  "${RFFS_ROOT}/rffs_input_bridge.cpp")
target_include_directories(RFFS-Online-v5 PRIVATE
  "${RFFS_ROOT}" "${FBNEO_ROOT}/src/burner/libretro/libretro-common/include"
  "${SDL_ROOT}/include" "${IMGUI_ROOT}"
  "${GGPO_ROOT}" "${GGPO_ROOT}/src/include" "${GGPO_ROOT}/src/lib/ggpo")
target_compile_definitions(RFFS-Online-v5 PRIVATE
  WIN32_LEAN_AND_MEAN NOMINMAX SDL_MAIN_HANDLED
  _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
target_link_libraries(RFFS-Online-v5 PRIVATE
  imgui_rffs GGPO "${SDL_ROOT}/lib/x64/SDL3.lib"
  opengl32 user32 kernel32 shell32 ole32 gdi32 winmm
  ws2_32 xinput9_1_0 Cabinet)
add_custom_command(TARGET RFFS-Online-v5 POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${SDL_ROOT}/lib/x64/SDL3.dll" "$<TARGET_FILE_DIR:RFFS-Online-v5>")
