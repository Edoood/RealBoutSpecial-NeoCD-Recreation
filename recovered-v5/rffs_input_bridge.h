#pragma once
#include <SDL3/SDL.h>
#include <string>
namespace rffs::inputbridge {
bool Start(const std::wstring& executable);void Stop();void Update();bool Active();
bool Identity(SDL_JoystickID id,std::string& key,std::string& name);
int Service(unsigned long parent);bool SelfTest();
}