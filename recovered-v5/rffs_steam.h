#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace rffs::steam {
struct Room {uint64_t id;std::string name;};
bool Init();void Shutdown();void Tick();bool Active();bool Hosting();uint64_t RoomID();uint64_t Peer();const std::string& Status();std::string Name();std::string PeerName();
bool OverlayPending();bool OverlayVisible();bool OverlayReady();void OpenOverlay();std::vector<Room> Friends();void InviteFriend(uint64_t id);
void Create();void Search();void Join(uint64_t id);void Leave();void Invite();const std::vector<Room>& Rooms();
bool Send(const void* data,size_t size,bool reliable);bool Receive(std::vector<uint8_t>& bytes);
}