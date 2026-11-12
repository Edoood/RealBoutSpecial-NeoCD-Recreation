#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "rffs_steam.h"
#include <filesystem>
#include <stdexcept>
#include <cstring>
#include <cstdio>
namespace rffs::steam {
namespace {
constexpr int channel=23119;constexpr const char* game="rffs-neocd-ggpo-v1";
HMODULE dll;void *user,*utils,*match,*friends,*network;int overlayAction=0;ULONGLONG overlayActionAt=0,overlayHandoffUntil=0;bool overlayOpen=false;int overlayState=-1;bool active=false,initialized=false,hosting=false;uint64_t self=0,room=0,peer=0,call=0,pendingInvite=0;int operation=0,pipe=0;ULONGLONG began=0;std::string status="Steam desconectada";std::vector<Room> rooms;
void (*shutdown)();uint64_t (*userID)(void*);const char* (*persona)(void*);const char* (*friendName)(void*,uint64_t);
uint64_t (*create)(void*,int,int);uint64_t (*join)(void*,uint64_t);void (*leave)(void*,uint64_t);int (*count)(void*,uint64_t);uint64_t (*member)(void*,uint64_t,int);uint64_t (*owner)(void*,uint64_t);
uint64_t (*list)(void*);void (*filter)(void*,const char*,const char*,int);void (*distance)(void*,int);uint64_t (*at)(void*,int);bool (*set)(void*,uint64_t,const char*,const char*);const char* (*get)(void*,uint64_t,const char*);bool (*joinable)(void*,uint64_t,bool);
bool (*completed)(void*,uint64_t,bool*);bool (*result)(void*,uint64_t,void*,int,int,bool*);
bool (*send)(void*,uint64_t,const void*,uint32_t,int,int);bool (*available)(void*,uint32_t*,int);bool (*receive)(void*,void*,uint32_t,uint32_t*,uint64_t*,int);bool (*accept)(void*,uint64_t);bool (*close)(void*,uint64_t);bool (*relay)(void*,bool);void (*invite)(void*,uint64_t);bool (*overlayEnabled)(void*);void (*openOverlay)(void*,const char*);bool (*inviteUser)(void*,uint64_t,uint64_t);
struct Callback {int user,id;uint8_t* data;int size;};void (*dispatchInit)();void (*dispatchFrame)(int);bool (*dispatchNext)(int,Callback*);void (*dispatchFree)(int);
struct FriendGame {uint64_t game;uint32_t ip;uint16_t port,query;uint64_t lobby;};int (*friendCount)(void*,int);uint64_t (*friendAt)(void*,int,int);bool (*friendGame)(void*,uint64_t,FriendGame*);
void say(std::string s){status=std::move(s);printf("[Steam] %s\n",status.c_str());}
template<class T>void load(T& fn,const char* name){fn=reinterpret_cast<T>(GetProcAddress(dll,name));if(!fn)throw std::runtime_error("Biblioteca Steam incompatível");}
void start(uint64_t c,int op){call=c;operation=op;began=GetTickCount64();if(!c)say("A Steam recusou o pedido.");}
bool allowed(uint64_t id){if(!room||!id||id==self)return false;for(int i=0;i<count(match,room);++i)if(member(match,room,i)==id)return true;return false;}
void addRoom(uint64_t id){if(!id||strcmp(get(match,id,"game"),game)||strcmp(get(match,id,"build"),"rffs-v5-2"))return;for(auto& r:rooms)if(r.id==id)return;rooms.push_back({id,get(match,id,"name")});}
}
bool Active(){return active;}bool Hosting(){return hosting;}uint64_t RoomID(){return room;}uint64_t Peer(){return peer;}const std::string& Status(){return status;}const std::vector<Room>& Rooms(){return rooms;}
std::string Name(){return active?persona(friends):"";}std::string PeerName(){return active&&peer?friendName(friends,peer):"";}
bool Init(){if(active)return true;try{wchar_t path[32768];GetModuleFileNameW(nullptr,path,32768);auto file=std::filesystem::path(path).parent_path()/L"steam_api64.dll";dll=LoadLibraryExW(file.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);if(!dll)throw std::runtime_error("steam_api64.dll não encontrada");
 int (*init)(char*);load(init,"SteamAPI_InitFlat");load(shutdown,"SteamAPI_Shutdown");char error[1024]{};if(init(error)!=0)throw std::runtime_error("Abra a Steam e entre na conta.");initialized=true;
 void* (*getter)();
#define INTERFACE(v,n) load(getter,n);v=getter();if(!v)throw std::runtime_error("Interface Steam indisponível")
 INTERFACE(user,"SteamAPI_SteamUser_v023");INTERFACE(utils,"SteamAPI_SteamUtils_v011");INTERFACE(match,"SteamAPI_SteamMatchmaking_v009");INTERFACE(friends,"SteamAPI_SteamFriends_v018");INTERFACE(network,"SteamAPI_SteamNetworking_v006");
#undef INTERFACE
#define API(v,n) load(v,"SteamAPI_ISteam" n)
 API(userID,"User_GetSteamID");API(persona,"Friends_GetPersonaName");API(friendName,"Friends_GetFriendPersonaName");API(create,"Matchmaking_CreateLobby");API(join,"Matchmaking_JoinLobby");API(leave,"Matchmaking_LeaveLobby");API(count,"Matchmaking_GetNumLobbyMembers");API(member,"Matchmaking_GetLobbyMemberByIndex");API(owner,"Matchmaking_GetLobbyOwner");API(list,"Matchmaking_RequestLobbyList");API(filter,"Matchmaking_AddRequestLobbyListStringFilter");API(distance,"Matchmaking_AddRequestLobbyListDistanceFilter");API(at,"Matchmaking_GetLobbyByIndex");API(set,"Matchmaking_SetLobbyData");API(get,"Matchmaking_GetLobbyData");API(joinable,"Matchmaking_SetLobbyJoinable");API(completed,"Utils_IsAPICallCompleted");API(result,"Utils_GetAPICallResult");API(send,"Networking_SendP2PPacket");API(available,"Networking_IsP2PPacketAvailable");API(receive,"Networking_ReadP2PPacket");API(accept,"Networking_AcceptP2PSessionWithUser");API(close,"Networking_CloseP2PSessionWithUser");API(relay,"Networking_AllowP2PPacketRelay");API(invite,"Friends_ActivateGameOverlayInviteDialog");API(overlayEnabled,"Utils_IsOverlayEnabled");API(openOverlay,"Friends_ActivateGameOverlay");API(inviteUser,"Matchmaking_InviteUserToLobby");API(friendCount,"Friends_GetFriendCount");API(friendAt,"Friends_GetFriendByIndex");API(friendGame,"Friends_GetFriendGamePlayed");
#undef API
 load(dispatchInit,"SteamAPI_ManualDispatch_Init");load(dispatchFrame,"SteamAPI_ManualDispatch_RunFrame");load(dispatchNext,"SteamAPI_ManualDispatch_GetNextCallback");load(dispatchFree,"SteamAPI_ManualDispatch_FreeLastCallback");int (*getPipe)();load(getPipe,"SteamAPI_GetHSteamPipe");dispatchInit();pipe=getPipe();self=userID(user);if(!self)throw std::runtime_error("Conta Steam indisponível");relay(network,true);active=true;say("Steam conectada: "+Name());return true;
 }catch(const std::exception& e){std::string message=e.what();Shutdown();say(message);return false;}}
void Leave(){if(active){if(peer)close(network,peer);if(room)leave(match,room);}room=peer=call=0;operation=0;hosting=false;}
void Shutdown(){overlayAction=0;overlayHandoffUntil=0;overlayOpen=false;Leave();active=false;if(initialized){shutdown();initialized=false;}if(dll){FreeLibrary(dll);dll=nullptr;}}
void Create(){if(!active||call)return;Leave();start(create(match,2,2),1);say("Criando sala...");}
void Search(){if(!active||call)return;rooms.clear();filter(match,"game",game,0);filter(match,"build","rffs-v5-2",0);distance(match,3);start(list(match),2);say("Procurando salas...");}
void Join(uint64_t id){if(!active||call||!id)return;Leave();start(join(match,id),3);say("Entrando na sala...");}
bool OverlayPending(){return active&&(overlayAction!=0||GetTickCount64()<overlayHandoffUntil);}
bool OverlayVisible(){return active&&overlayOpen;}
bool OverlayReady(){return active&&overlayEnabled&&overlayEnabled(utils);}
void OpenOverlay(){if(!active)return;if(OverlayReady()){overlayAction=1;overlayActionAt=GetTickCount64();say("Overlay solicitado pelo botão.");}else say("Overlay indisponível. Use a lista de amigos abaixo para convidar.");}
std::vector<Room> Friends(){std::vector<Room> list;if(active)for(int i=0;i<friendCount(friends,4)&&i<250;++i){auto id=friendAt(friends,i,4);list.push_back({id,friendName(friends,id)});}return list;}
void InviteFriend(uint64_t id){if(!active||!room||!id)return;if(inviteUser(match,room,id))say("Convite Steam enviado para "+std::string(friendName(friends,id))+".");else say("A Steam recusou o convite. Tente novamente.");}
void Invite(){if(active&&room){if(OverlayReady()){overlayAction=2;overlayActionAt=GetTickCount64();say("Convite solicitado pelo botão.");}else say("Overlay indisponível. Escolha o amigo na lista abaixo.");}}
void Tick(){if(!active)return;int currentOverlay=OverlayReady()?1:0;if(currentOverlay!=overlayState){overlayState=currentOverlay;printf("[Steam] Overlay disponível: %s\n",currentOverlay?"sim":"não");}dispatchFrame(pipe);Callback c{};while(dispatchNext(pipe,&c)){if(c.id==331&&c.data&&c.size>=1){overlayOpen=c.data[0]!=0;overlayHandoffUntil=0;printf("[Steam] Overlay ativo=%d iniciado_pelo_usuario=%d\n",overlayOpen,c.size>=2?c.data[1]:0);}if(c.id==333&&c.data&&c.size>=16)memcpy(&pendingInvite,c.data,8);if(c.id==1202&&c.data&&c.size==8){uint64_t id;memcpy(&id,c.data,8);if(allowed(id))accept(network,id);}if(c.id==1203)say("Falha na conexão P2P Steam. Tente entrar novamente.");dispatchFree(pipe);}
 if(overlayAction&&GetTickCount64()-overlayActionAt>=150){int action=overlayAction;overlayAction=0;overlayHandoffUntil=GetTickCount64()+1500;if(OverlayReady()){if(action==2&&room)invite(friends,room);else if(action==1)openOverlay(friends,"Friends");}}
 if(pendingInvite&&!call){auto id=pendingInvite;pendingInvite=0;Join(id);}
 if(call){bool failed=false;if(GetTickCount64()-began>20000){call=0;operation=0;say("A Steam demorou para responder. Tente novamente.");}else if(completed(utils,call,&failed)){auto op=operation;auto handle=call;call=0;operation=0;bool ok=false;
  if(op==1){struct Reply{int result;uint64_t id;}v{};ok=!failed&&result(utils,handle,&v,sizeof(v),513,&failed)&&!failed&&v.result==1;if(ok){room=v.id;hosting=true;set(match,room,"game",game);set(match,room,"build","rffs-v5-2");set(match,room,"name",Name().c_str());joinable(match,room,true);say("Sala criada. Convide um amigo ou aguarde a busca.");}}
  if(op==2){struct Reply{uint32_t count;}v{};ok=!failed&&result(utils,handle,&v,sizeof(v),510,&failed)&&!failed;if(ok){for(unsigned i=0;i<v.count&&i<200;++i)addRoom(at(match,i));for(int i=0;i<friendCount(friends,4);++i){FriendGame fg{};if(friendGame(friends,friendAt(friends,i,4),&fg))addRoom(fg.lobby);}say(std::to_string(rooms.size())+" sala(s) encontrada(s).");}}
  if(op==3){struct Reply{uint64_t id;uint32_t permissions;bool locked;uint32_t response;}v{};ok=!failed&&result(utils,handle,&v,sizeof(v),504,&failed)&&!failed&&v.response==1;if(ok){room=v.id;hosting=false;if(strcmp(get(match,room,"game"),game)||strcmp(get(match,room,"build"),"rffs-v5-2")){Leave();ok=false;}else say("Sala conectada. Aguardando o anfitrião iniciar.");}}
  if(!ok)say("Não foi possível abrir essa sala. Confira as versões.");}}
 if(room){if(count(match,room)<1){Leave();say("Sala encerrada.");return;}hosting=owner(match,room)==self;uint64_t found=0;for(int i=0;i<count(match,room);++i){auto id=member(match,room,i);if(id!=self){found=id;break;}}if(found!=peer){if(peer)close(network,peer);peer=found;if(peer){accept(network,peer);say("Amigo conectado: "+PeerName());}}}
}
bool Send(const void* bytes,size_t size,bool reliable){return active&&peer&&size&&size<=65536&&send(network,peer,bytes,uint32_t(size),reliable||size>1200?2:0,channel);}
bool Receive(std::vector<uint8_t>& bytes){if(!active)return false;uint32_t size=0;for(int i=0;i<256&&available(network,&size,channel);++i){if(size>65536){std::vector<uint8_t> drop(size<=1048576?size:0);if(drop.empty())return false;uint32_t n;uint64_t who;receive(network,drop.data(),size,&n,&who,channel);continue;}bytes.resize(size);uint32_t n=0;uint64_t who=0;if(!receive(network,bytes.data(),size,&n,&who,channel))return false;if(who==peer&&allowed(who)&&n<=size){bytes.resize(n);return true;}}return false;}
}