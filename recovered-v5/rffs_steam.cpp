#include "rffs_steam.h"

namespace rffs::steam {
bool Init() { return false; }
void Shutdown() {}
void Tick() {}
bool Active() { return false; }
bool Hosting() { return false; }
uint64_t RoomID() { return 0; }
uint64_t Peer() { return 0; }
const std::string& Status() {
    static const std::string status = "Edição offline";
    return status;
}
std::string Name() { return {}; }
std::string PeerName() { return {}; }
bool OverlayPending() { return false; }
bool OverlayVisible() { return false; }
bool OverlayReady() { return false; }
void OpenOverlay() {}
std::vector<Room> Friends() { return {}; }
void InviteFriend(uint64_t) {}
void Create() {}
void Search() {}
void Join(uint64_t) {}
void Leave() {}
void Invite() {}
const std::vector<Room>& Rooms() {
    static const std::vector<Room> rooms;
    return rooms;
}
bool Send(const void*, size_t, bool) { return false; }
bool Receive(std::vector<uint8_t>&) { return false; }
}
