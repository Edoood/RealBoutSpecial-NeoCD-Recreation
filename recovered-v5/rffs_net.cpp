#include "rffs_net.h"
#include "rffs_steam.h"
#include <windows.h>
#include <compressapi.h>
#include "imgui/imgui.h"
#include "ggpo/src/include/ggponet.h"
#include "ggpo/src/lib/ggpo/backends/p2p.h"
#include <algorithm>
#include <deque>
#include <map>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
namespace rffs::net {
namespace {
Core core;std::vector<uint8_t> initial,offline,transfer;uint64_t key=0,nonce=0,expectedPeer=0;GGPOSession* session=nullptr;GGPOPlayerHandle localHandle=0,remoteHandle=0;bool running=false,test=false;int phase=0,local=0,delay=2,skip=0,frame=0,rollbacks=0;uint32_t sent=0,received=0,acknowledged=0;uint64_t transferHash=0;bool endSent=false,peerReady=false;uint64_t lastHello=0,lastProgress=0;uint64_t began=0;std::string status="Crie uma sala ou procure um amigo.",failure;std::deque<std::vector<uint8_t>> incoming;std::map<int,uint64_t> hashes,theirs;int lastHash=-1;
#pragma pack(push,1)
struct Wire {uint32_t magic=0x52464735;uint16_t type=0,version=2;uint64_t nonce=0;uint32_t a=0,b=0;uint64_t value=0;};
#pragma pack(pop)
struct Manifest {uint64_t hash;uint32_t rawSize,codec;};
Manifest manifest{};
std::string lastError;uint64_t errorUntil=0;
uint64_t fingerprint(const uint8_t* p,size_t n){uint64_t h=14695981039346656037ull;for(size_t i=0;i<n;++i){h^=p[i];h*=1099511628211ull;}return h;}
std::vector<uint8_t> compressState(const std::vector<uint8_t>& raw,uint32_t& codec){
 codec=0;COMPRESSOR_HANDLE h=nullptr;if(!CreateCompressor(COMPRESS_ALGORITHM_XPRESS_HUFF,nullptr,&h))return raw;
 SIZE_T needed=0;Compress(h,raw.data(),raw.size(),nullptr,0,&needed);std::vector<uint8_t> zipped;
 if(needed&&needed<=raw.size()+1024*1024){zipped.resize(needed);if(Compress(h,raw.data(),raw.size(),zipped.data(),zipped.size(),&needed)){zipped.resize(needed);codec=zipped.size()<raw.size()?1:0;}else zipped.clear();}CloseCompressor(h);return codec?zipped:raw;
}
std::vector<uint8_t> decodeState(const std::vector<uint8_t>& encoded,const Manifest& m){
 if(!m.rawSize||m.rawSize>32*1024*1024)throw std::runtime_error("Tamanho do estado inválido.");
 std::vector<uint8_t> raw;if(m.codec==0)raw=encoded;else if(m.codec==1){DECOMPRESSOR_HANDLE h=nullptr;if(!CreateDecompressor(COMPRESS_ALGORITHM_XPRESS_HUFF,nullptr,&h))throw std::runtime_error("Falha ao preparar descompressão.");raw.resize(m.rawSize);SIZE_T got=0;bool ok=Decompress(h,encoded.data(),encoded.size(),raw.data(),raw.size(),&got)!=0;CloseDecompressor(h);if(!ok||got!=m.rawSize)throw std::runtime_error("Dados comprimidos inválidos.");}else throw std::runtime_error("Formato de estado desconhecido.");
 if(raw.size()!=m.rawSize||fingerprint(raw.data(),raw.size())!=m.hash)throw std::runtime_error("Verificação do estado inicial falhou.");return raw;
}
bool sameInitial(const Manifest& m){return initial.size()==m.rawSize&&fingerprint(initial.data(),initial.size())==m.hash;}
void say(std::string s){status=std::move(s);printf("[Netplay] %s\n",status.c_str());}
bool packet(int type,uint32_t a=0,uint32_t b=0,uint64_t value=0,const void* data=nullptr,size_t n=0,bool reliable=true){Wire w;w.type=uint16_t(type);w.nonce=nonce;w.a=a;w.b=b;w.value=value;std::vector<uint8_t> bytes(sizeof(w)+n);memcpy(bytes.data(),&w,sizeof(w));if(n)memcpy(bytes.data()+sizeof(w),data,n);return steam::Send(bytes.data(),bytes.size(),reliable);}
bool hello(){return packet(1,uint32_t(transfer.size()),delay,key,&manifest,sizeof(manifest));}
bool begin(const char*){return true;}
bool save(unsigned char** buffer,int* length,int* checksum,int at){auto bytes=core.save();*length=int(bytes.size())+4;*buffer=static_cast<unsigned char*>(malloc(*length));if(!*buffer)return false;memcpy(*buffer,&at,4);memcpy(*buffer+4,bytes.data(),bytes.size());*checksum=int(core.hash());if(at%60==0)hashes[at]=core.hash();return true;}
bool load(unsigned char* buffer,int length){if(length<4)return false;memcpy(&frame,buffer,4);return core.load(std::vector<uint8_t>(buffer+4,buffer+length));}
bool logState(char* name,unsigned char* bytes,int n){FILE* f=nullptr;fopen_s(&f,name,"wb");if(!f)return false;fwrite(bytes,1,n,f);fclose(f);return true;}
void freeBuffer(void* p){free(p);}
bool advance(int){uint16_t input[2]{};int disconnected=0;if(ggpo_synchronize_input(session,input,sizeof(input),&disconnected)!=GGPO_OK)return false;core.run(input,true);++frame;++rollbacks;return ggpo_advance_frame(session)==GGPO_OK;}
bool event(GGPOEvent* e){switch(e->code){case GGPO_EVENTCODE_RUNNING:running=true;say("Versus conectado • GGPO ativo");break;case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:say("Sincronizando GGPO...");break;case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:failure="O outro jogador desconectou.";break;case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:say("Aguardando conexão do outro jogador...");break;case GGPO_EVENTCODE_CONNECTION_RESUMED:say("Conexão retomada.");break;case GGPO_EVENTCODE_TIMESYNC:skip=std::min(e->u.timesync.frames_ahead,6);break;default:break;}return true;}
GGPOSessionCallbacks callbacks(){return {begin,save,load,logState,freeBuffer,advance,event};}
void startGGPO(){auto cb=callbacks();frame=rollbacks=skip=0;hashes.clear();theirs.clear();lastHash=-1;running=false;auto rc=ggpo_start_session(&session,&cb,"rffs-neocd",2,sizeof(uint16_t),local?7001:7000);if(rc!=GGPO_OK)throw std::runtime_error("Falha ao iniciar GGPO");for(int p=0;p<2;++p){GGPOPlayer player{};player.size=sizeof(player);player.player_num=p+1;player.type=p==local?GGPO_PLAYERTYPE_LOCAL:GGPO_PLAYERTYPE_REMOTE;strcpy_s(player.u.remote.ip_address,"127.0.0.1");player.u.remote.port=p?7001:7000;GGPOPlayerHandle h;if(ggpo_add_player(session,&player,&h)!=GGPO_OK)throw std::runtime_error("Falha ao configurar jogador GGPO");if(p==local)localHandle=h;else remoteHandle=h;}ggpo_set_frame_delay(session,localHandle,delay);ggpo_set_disconnect_timeout(session,12000);ggpo_set_disconnect_notify_start(session,2000);phase=3;began=GetTickCount64();say("Sincronizando GGPO...");}
void hostStart(){errorUntil=0;if(!steam::Hosting()||!steam::Peer()||phase)return;expectedPeer=steam::Peer();offline=core.save();manifest.hash=fingerprint(initial.data(),initial.size());manifest.rawSize=uint32_t(initial.size());transfer=compressState(initial,manifest.codec);transferHash=manifest.hash;printf("[Netplay] Estado inicial: %zu -> %zu bytes, codec=%u\n",initial.size(),transfer.size(),manifest.codec);endSent=false;peerReady=false;lastHello=lastProgress=0;local=0;nonce=(GetTickCount64()<<16)^steam::RoomID();if(!nonce)nonce=1;phase=1;sent=acknowledged=0;began=GetTickCount64();incoming.clear();if(!hello()){failure="Falha ao iniciar comunicação Steam.";return;}say("Preparando a partida para os dois jogadores...");}
void compare(){for(auto it=theirs.begin();it!=theirs.end();){auto ours=hashes.find(it->first);if(ours==hashes.end()){++it;continue;}if(ours->second!=it->second){failure="Dessincronização detectada no quadro "+std::to_string(it->first)+". Partida encerrada.";return;}it=theirs.erase(it);}while(hashes.size()>256)hashes.erase(hashes.begin());while(theirs.size()>256)theirs.erase(theirs.begin());}
}
void SetInitial(const std::vector<uint8_t>& state){if(!phase)initial=state;}
void Init(Core c,std::vector<uint8_t> state,uint64_t compatibility){core=std::move(c);initial=std::move(state);key=compatibility;srand(unsigned(GetTickCount64()^GetCurrentProcessId()));}
bool Locked(){return phase!=0;}const std::string& Status(){return status;}
void Leave(){if(phase&&steam::Peer())packet(7);if(session){ggpo_close_session(session);session=nullptr;}phase=0;running=false;incoming.clear();transfer.clear();hashes.clear();theirs.clear();if(!offline.empty()){core.load(offline);offline.clear();}steam::Leave();say("Partida encerrada. Voltou ao jogo local.");failure.clear();}
void Shutdown(){Leave();steam::Shutdown();}
void Tick(){steam::Tick();try{std::vector<uint8_t> data;for(int budget=0;budget<512&&steam::Receive(data);++budget){if(data.size()<sizeof(Wire))continue;Wire w;memcpy(&w,data.data(),sizeof(w));if(w.magic!=0x52464735||w.version!=2)continue;auto* bytes=data.data()+sizeof(w);size_t n=data.size()-sizeof(w);
 if(w.type==1&&!phase&&!steam::Hosting()){Manifest offered{};if(n==sizeof(offered))memcpy(&offered,bytes,sizeof(offered));if(w.value!=key||!w.a||w.a>32*1024*1024||w.b>4||n!=sizeof(offered)||offered.rawSize!=initial.size()||offered.codec>1){lastError="Versões do jogo, BIOS ou disco diferentes. Sala recusada.";errorUntil=GetTickCount64()+15000;say(lastError);steam::Leave();continue;}expectedPeer=steam::Peer();offline=core.save();manifest=offered;transferHash=manifest.hash;nonce=w.nonce;delay=int(w.b);local=1;received=0;began=GetTickCount64();incoming.clear();if(sameInitial(manifest)){if(!core.load(initial))throw std::runtime_error("Falha ao carregar estado inicial local.");packet(10,0,0,manifest.hash);printf("[Netplay] Estado local idêntico: transferência dispensada\n");startGGPO();}else{transfer.assign(w.a,0);phase=2;packet(8);say("Recebendo preparação comprimida...");}continue;}
 if(!phase||w.nonce!=nonce)continue;
 if(w.type==1&&phase==3&&local==1){packet(10,0,0,transferHash);continue;}
 if(w.type==10&&phase==1&&w.value==transferHash){if(!core.load(initial))throw std::runtime_error("Falha ao carregar estado inicial local.");printf("[Netplay] Amigo confirmou estado idêntico: zero bytes de estado enviados\n");startGGPO();transfer.clear();continue;}
 if(w.type==1&&phase==2){packet(8);continue;}
 if(w.type==8&&phase==1){peerReady=true;say("Conexão confirmada. Enviando dados da partida...");}
 if(w.type==2&&phase==2&&w.a==received&&w.b==n&&n&&received+n<=transfer.size()){memcpy(transfer.data()+received,bytes,n);received+=uint32_t(n);packet(9,received);began=GetTickCount64();}
 if(w.type==9&&phase==1&&w.a<=sent&&w.a>acknowledged){acknowledged=w.a;began=GetTickCount64();}
 if(w.type==3&&phase==2){if(received!=transfer.size()||w.value!=manifest.hash||!core.load(decodeState(transfer,manifest))){failure="Falha ao sincronizar o estado inicial.";continue;}transferHash=w.value;packet(4,0,0,w.value);startGGPO();transfer.clear();}
 if(w.type==3&&phase==3&&local==1&&w.value==transferHash){packet(4,0,0,transferHash);continue;}
 if(w.type==4&&phase==1&&sent==transfer.size()&&transferHash==w.value){if(!core.load(initial)){failure="Falha ao carregar partida inicial.";continue;}startGGPO();transfer.clear();}
 if(w.type==5&&n>=5&&n<=4096&&incoming.size()<512)incoming.emplace_back(bytes,bytes+n);
 if(w.type==6&&phase==3)theirs[int(w.a)]=w.value;
 if(w.type==7)failure="O outro jogador saiu da partida.";
 }
 if(phase==1&&!peerReady&&GetTickCount64()-lastHello>=1000){lastHello=GetTickCount64();hello();}
 if(phase==1&&peerReady&&!transfer.empty()){for(int budget=0;budget<2&&sent<transfer.size()&&sent-acknowledged<262144;++budget){uint32_t n=uint32_t(std::min<size_t>(32768,transfer.size()-sent));if(!packet(2,sent,n,0,transfer.data()+sent,n))break;sent+=n;}if(acknowledged==transfer.size()&&(!endSent||GetTickCount64()-lastHello>=1000)){lastHello=GetTickCount64();endSent=packet(3,0,0,transferHash);}}
 if((phase==1||phase==2)&&GetTickCount64()-lastProgress>=1000){lastProgress=GetTickCount64();if(phase==1&&!peerReady)say("Aguardando resposta do amigo pela rede Steam...");else if(phase==1&&acknowledged==transfer.size())say("Dados recebidos. Aguardando o amigo carregar a partida...");else say(std::string(phase==1?"Enviando partida: ":"Recebendo partida: ")+std::to_string((phase==1?acknowledged:received)*100ull/std::max<size_t>(1,transfer.size()))+"%");}
 if(phase&&(!steam::RoomID()||!steam::Peer()||steam::Peer()!=expectedPeer))failure="O outro jogador saiu da sala.";
 if(phase&&phase!=3&&GetTickCount64()-began>60000)failure="A conexão demorou demais. Entre novamente na sala.";
 if(phase==3&&!running&&GetTickCount64()-began>30000)failure="O GGPO não conseguiu sincronizar. Entre novamente na sala.";
 if(session){ggpo_idle(session,0);if(phase==3&&!test){int confirmed=static_cast<Peer2PeerBackend*>(session)->ConfirmedFrame();auto it=hashes.upper_bound(confirmed);if(it!=hashes.begin()){--it;if(it->first>lastHash){packet(6,uint32_t(it->first),0,it->second);lastHash=it->first;}}compare();}}
 if(!failure.empty()){auto message=failure;Leave();lastError=message;errorUntil=GetTickCount64()+15000;say(message);}
 }catch(const std::exception& e){std::string message=e.what();Leave();lastError=message;errorUntil=GetTickCount64()+15000;say(message);}}
bool Step(uint16_t input){if(!session||!running)return false;if(skip>0){--skip;return false;}auto rc=ggpo_add_local_input(session,localHandle,&input,sizeof(input));if(rc==GGPO_ERRORCODE_PREDICTION_THRESHOLD||rc==GGPO_ERRORCODE_NOT_SYNCHRONIZED)return false;if(rc!=GGPO_OK){failure="Falha ao enviar comando GGPO.";return false;}uint16_t both[2]{};int disconnected=0;if(ggpo_synchronize_input(session,both,sizeof(both),&disconnected)!=GGPO_OK)return false;if(disconnected){failure="O outro jogador desconectou.";return false;}core.run(both,false);++frame;return ggpo_advance_frame(session)==GGPO_OK;}
void Draw(){ImGui::TextWrapped("%s",steam::Status().c_str());ImGui::Spacing();if(!steam::Active()){if(ImGui::Button("Conectar Steam"))steam::Init();return;}ImGui::Text("Sua conta: %s",steam::Name().c_str());ImGui::TextDisabled("Overlay Steam: %s",steam::OverlayReady()?"disponível":"indisponível");if(ImGui::Button("Abrir overlay Steam"))steam::OpenOverlay();ImGui::TextWrapped("%s",status.c_str());if(phase){if(ImGui::Button("Sair da partida"))Leave();return;}if(steam::RoomID()){ImGui::Text("Sala: %llu",static_cast<unsigned long long>(steam::RoomID()));if(steam::Peer())ImGui::Text("Amigo: %s",steam::PeerName().c_str());else ImGui::TextDisabled("Aguardando outro jogador...");if(ImGui::Button("Convidar amigo (overlay)"))steam::Invite();
 static uint64_t inviteID=0;static std::string inviteName="Escolha seu amigo";if(ImGui::BeginCombo("Amigo Steam",inviteName.c_str())){for(const auto& f:steam::Friends()){ImGui::PushID(std::to_string(f.id).c_str());if(ImGui::Selectable(f.name.c_str(),inviteID==f.id)){inviteID=f.id;inviteName=f.name;}ImGui::PopID();}ImGui::EndCombo();}ImGui::BeginDisabled(!inviteID);if(ImGui::Button("Enviar convite Steam"))steam::InviteFriend(inviteID);ImGui::EndDisabled();if(steam::Hosting()){ImGui::SliderInt("Atraso de entrada",&delay,0,4,"%d quadros");ImGui::BeginDisabled(!steam::Peer());if(ImGui::Button("Iniciar versus"))hostStart();ImGui::EndDisabled();}if(ImGui::Button("Sair da sala"))steam::Leave();return;}if(ImGui::Button("Criar sala"))steam::Create();ImGui::SameLine();if(ImGui::Button("Procurar salas"))steam::Search();for(const auto& room:steam::Rooms()){ImGui::PushID(std::to_string(room.id).c_str());if(ImGui::Button("Entrar"))steam::Join(room.id);ImGui::SameLine();ImGui::TextUnformatted(room.name.c_str());ImGui::PopID();}ImGui::Spacing();ImGui::TextWrapped("Cada pessoa usa seus controles de Jogador 1. O anfitrião será P1 e o convidado P2. Após iniciar, feche o F1 e pressione Start para entrar no jogo.");}
void HUD(){if(!phase&&GetTickCount64()>=errorUntil)return;ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x*.5f,12),ImGuiCond_Always,ImVec2(.5f,0));ImGui::SetNextWindowBgAlpha(.75f);ImGui::Begin("GGPO",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoSavedSettings);if(session&&running){GGPONetworkStats stats{};ggpo_get_network_stats(session,remoteHandle,&stats);if(stats.network.ping>0){auto color=stats.network.ping<=80?ImVec4(.35f,1,.65f,1):(stats.network.ping<=150?ImVec4(1,.85f,.3f,1):ImVec4(1,.4f,.35f,1));ImGui::TextColored(color,"PING %d ms",stats.network.ping);}else ImGui::TextUnformatted("Medindo ping...");ImGui::TextDisabled("P%d | Delay %d | GGPO",local+1,delay);}else ImGui::TextUnformatted(phase?status.c_str():lastError.c_str());ImGui::End();}
bool PreparationSelfTest(){
 auto before=core.save();auto beforeHash=core.hash();Manifest m{fingerprint(initial.data(),initial.size()),uint32_t(initial.size()),0};
 auto zipped=compressState(initial,m.codec);auto decoded=decodeState(zipped,m);if(decoded!=initial||!sameInitial(m)||!core.load(decoded))return false;
 auto wrong=m;wrong.hash^=1;if(sameInitial(wrong))return false;bool rejected=false;try{decodeState(zipped,wrong);}catch(...){rejected=true;}if(!rejected)return false;
 if(!core.load(before)||core.hash()!=beforeHash)return false;
 printf("PREPARATION_PASS raw=%zu compressed=%zu codec=%u local-identical=skip-transfer mismatch=fallback checksum=rejected restore=ok\n",initial.size(),zipped.size(),m.codec);return true;
}
bool SelfTest(int count){auto backup=core.save();test=true;phase=3;running=false;auto cb=callbacks();if(ggpo_start_synctest(&session,&cb,"rffs-neocd",2,sizeof(uint16_t),6)!=GGPO_OK)return false;ggpo_idle(session,0);for(int i=0;i<count;++i){uint16_t input=uint16_t((i%120<4?1u<<3:0)|(i%19<3?1u:0)|(i%120<60?1u<<7:1u<<6));ggpo_add_local_input(session,0,&input,sizeof(input));ggpo_add_local_input(session,1,&input,sizeof(input));uint16_t both[2];int flags;ggpo_synchronize_input(session,both,sizeof(both),&flags);core.run(both,false);++frame;ggpo_advance_frame(session);}ggpo_close_session(session);session=nullptr;core.load(backup);phase=0;test=false;printf("GGPO_SELFTEST_PASS frames=%d replays=%d\n",count,rollbacks);return rollbacks>=count;}
}
extern "C" bool rffs_ggpo_send(const void* bytes,int n){return n>0&&n<=4096&&rffs::net::packet(5,0,0,0,bytes,size_t(n),false);}
extern "C" int rffs_ggpo_recv(void* bytes,int capacity){auto& queue=rffs::net::incoming;if(queue.empty())return 0;auto data=std::move(queue.front());queue.pop_front();if(data.size()>size_t(capacity))return 0;memcpy(bytes,data.data(),data.size());return int(data.size());}
extern "C" int rffs_ggpo_remote_port(){return rffs::net::local?7000:7001;}