#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include <string>
namespace rffs::net {
struct Core {std::function<std::vector<uint8_t>()> save;std::function<bool(const std::vector<uint8_t>&)> load;std::function<void(const uint16_t*,bool)> run;std::function<uint64_t()> hash;};
void SetInitial(const std::vector<uint8_t>& state);
void Init(Core core,std::vector<uint8_t> initial,uint64_t compatibility);void Tick();bool Step(uint16_t buttons);void Leave();void Shutdown();bool Locked();void Draw();void HUD();bool PreparationSelfTest();bool SelfTest(int frames);const std::string& Status();
}