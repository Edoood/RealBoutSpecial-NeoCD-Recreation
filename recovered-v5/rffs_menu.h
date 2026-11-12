#pragma once
#include <SDL3/SDL.h>
#include <string>
namespace rffs::menu {
struct Settings {bool directBoot=true,fullscreen=true,widescreen=false;int filter=2;float smoothing=.8f,brightness=1.f,volume=.85f;};
extern Settings settings;
extern bool visible,quitRequested,restartRequested,saveStateRequested,loadStateRequested;
void Init(SDL_Window* window,const std::wstring& configPath,bool testMode=false);
void ApplyWindowMode(bool borderless);
void Shutdown();void Process(const SDL_Event& event);void Update();void Draw(bool gpuAvailable);
int Input(unsigned port,unsigned libretroId);
void Notify(const std::string& message);
void AdvanceInput();void ShortcutHUD();
void Save();void SetTab(int tab);
bool InputSelfTest();
}