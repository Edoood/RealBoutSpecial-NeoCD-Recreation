#pragma once
#include <SDL3/SDL.h>
#include <cstdint>
namespace rffs::gpu {
bool Init(SDL_Window* window);
void Draw(const uint32_t* pixels,int w,int h,int mode,float smoothing,float brightness);
void Swap();void Shutdown();void Capture(const char* path);
}