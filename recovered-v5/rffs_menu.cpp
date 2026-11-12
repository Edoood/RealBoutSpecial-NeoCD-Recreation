#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <Xinput.h>
#include "rffs_menu.h"
#include "rffs_net.h"
#include "rffs_steam.h"
#include "rffs_input_bridge.h"
#include "imgui/imgui.h"
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdio>
namespace rffs::menu {
Settings settings;bool visible=false,quitRequested=false,restartRequested=false,saveStateRequested=false,loadStateRequested=false;
static SDL_Window* win;static std::wstring path;static bool testMode=false,dirty=false;static Uint64 lastSave=0,blockUntil=0;
static std::array<bool,SDL_SCANCODE_COUNT> keys{};
static std::string notice;static Uint64 noticeUntil=0;
void Notify(const std::string& message){notice=message;noticeUntil=SDL_GetTicks()+4000;}
static int exFrame=-1;static bool exUnlock=false;static Uint64 exNoticeUntil=0;
static void selectEX(){exUnlock=false;exFrame=0;exNoticeUntil=SDL_GetTicks()+5000;}
static int tab=0,capturePort=-1,captureAction=-1,captureKind=0;static bool captureArmed=false;
static DWORD (WINAPI* getXInput)(DWORD,XINPUT_STATE*)=nullptr;static HMODULE xinputModule=nullptr;
static const char* actionNames[]={"Cima","Baixo","Esquerda","Direita","Botão A","Botão B","Botão C","Botão D","Start","Select"};
struct Binding {int type=0,index=0,value=0;}; // pad button, raw button, hat, signed axis, XInput mask
struct Player {int mode=0,xindex=0;bool keyboard=true;std::string device;std::array<int,10> key{};std::array<Binding,10> bind{};};
static std::array<Player,2> players;
struct Device {SDL_JoystickID id;SDL_Gamepad* pad;SDL_Joystick* stick;std::string name,key;};
static std::vector<Device> devices;
static std::wstring wide(const std::string& s){int n=MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,nullptr,0);std::wstring w(n,0);MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,w.data(),n);if(n)w.pop_back();return w;}
static std::string narrow(const std::wstring& s){int n=WideCharToMultiByte(CP_UTF8,0,s.c_str(),-1,nullptr,0,nullptr,nullptr);std::string v(n,0);WideCharToMultiByte(CP_UTF8,0,s.c_str(),-1,v.data(),n,nullptr,nullptr);if(n)v.pop_back();return v;}
static std::string read(const std::string& key,const std::string& fallback){wchar_t b[2048];GetPrivateProfileStringW(L"RealBout",wide(key).c_str(),wide(fallback).c_str(),b,2048,path.c_str());return narrow(b);}
static int integer(const std::string& key,int fallback){try{return std::stoi(read(key,std::to_string(fallback)));}catch(...){return fallback;}}
static float number(const std::string& key,float fallback){try{return std::stof(read(key,std::to_string(fallback)));}catch(...){return fallback;}}
static void write(const std::string& key,const std::string& value){WritePrivateProfileStringW(L"RealBout",wide(key).c_str(),wide(value).c_str(),path.c_str());}
static void defaults(){
 players={};const int p1[]={SDL_SCANCODE_UP,SDL_SCANCODE_DOWN,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT,SDL_SCANCODE_Z,SDL_SCANCODE_X,SDL_SCANCODE_C,SDL_SCANCODE_V,SDL_SCANCODE_RETURN,SDL_SCANCODE_RSHIFT};
 const int p2[]={SDL_SCANCODE_W,SDL_SCANCODE_S,SDL_SCANCODE_A,SDL_SCANCODE_D,SDL_SCANCODE_F,SDL_SCANCODE_G,SDL_SCANCODE_R,SDL_SCANCODE_T,SDL_SCANCODE_TAB,SDL_SCANCODE_LSHIFT};
 const int buttons[]={SDL_GAMEPAD_BUTTON_DPAD_UP,SDL_GAMEPAD_BUTTON_DPAD_DOWN,SDL_GAMEPAD_BUTTON_DPAD_LEFT,SDL_GAMEPAD_BUTTON_DPAD_RIGHT,SDL_GAMEPAD_BUTTON_SOUTH,SDL_GAMEPAD_BUTTON_EAST,SDL_GAMEPAD_BUTTON_WEST,SDL_GAMEPAD_BUTTON_NORTH,SDL_GAMEPAD_BUTTON_START,SDL_GAMEPAD_BUTTON_BACK};
 for(int p=0;p<2;++p){players[p].xindex=p;for(int a=0;a<10;++a){players[p].key[a]=p?p2[a]:p1[a];players[p].bind[a]={0,buttons[a],0};}}
}
void Save(){if(path.empty())return;write("DirectBoot",std::to_string(settings.directBoot));write("BorderlessWindow",std::to_string(settings.fullscreen));write("Widescreen",std::to_string(settings.widescreen));write("Filter",std::to_string(settings.filter));write("Smoothing",std::to_string(settings.smoothing));write("Brightness",std::to_string(settings.brightness));write("Volume",std::to_string(settings.volume));
 for(int p=0;p<2;++p){auto prefix="P"+std::to_string(p+1);auto& v=players[p];write(prefix+"Mode",std::to_string(v.mode));write(prefix+"Device",v.device);write(prefix+"XInput",std::to_string(v.xindex));write(prefix+"Keyboard",std::to_string(v.keyboard));for(int a=0;a<10;++a){write(prefix+"Key"+std::to_string(a),std::to_string(v.key[a]));const auto& b=v.bind[a];write(prefix+"Bind"+std::to_string(a),std::to_string(b.type)+","+std::to_string(b.index)+","+std::to_string(b.value));}}
 dirty=false;lastSave=SDL_GetTicks();
}
static void refresh(){
 for(auto& d:devices){if(d.pad)SDL_CloseGamepad(d.pad);else if(d.stick)SDL_CloseJoystick(d.stick);}devices.clear();int count=0;auto* ids=SDL_GetJoysticks(&count);std::map<std::string,int> instances;
 for(int i=0;i<count;++i){std::string bridgeKey,bridgeName;if(rffs::inputbridge::Active()&&!rffs::inputbridge::Identity(ids[i],bridgeKey,bridgeName))continue;Device d{};d.id=ids[i];if(SDL_IsGamepad(d.id)){d.pad=SDL_OpenGamepad(d.id);if(d.pad)d.stick=SDL_GetGamepadJoystick(d.pad);}else d.stick=SDL_OpenJoystick(d.id);if(!d.stick&&!d.pad)d.stick=SDL_OpenJoystick(d.id);if(!d.stick){printf("INPUT open failed id=%u: %s\n",d.id,SDL_GetError());continue;}
  const char* name=SDL_GetJoystickName(d.stick);d.name=name?name:"Controle sem nome";char guid[64];SDL_GUIDToString(SDL_GetJoystickGUID(d.stick),guid,sizeof(guid));std::string base=guid;const char* serial=SDL_GetJoystickSerial(d.stick);if(serial&&*serial)base+="/"+std::string(serial);d.key=base+"#"+std::to_string(instances[base]++);if(!bridgeKey.empty()){d.key=bridgeKey;d.name=bridgeName;}printf("INPUT detected: %s gamepad=%d id=%u\n",d.name.c_str(),d.pad!=nullptr,d.id);devices.push_back(std::move(d));}
 SDL_free(ids);
}
void Init(SDL_Window* window,const std::wstring& configPath,bool testing){win=window;path=configPath;testMode=testing;defaults();settings.widescreen=integer("Widescreen",0)!=0;settings.directBoot=integer("DirectBoot",1)!=0;settings.fullscreen=integer("BorderlessWindow",1)!=0;settings.filter=std::clamp(integer("Filter",2),0,3);settings.smoothing=std::clamp(number("Smoothing",.8f),0.f,1.f);settings.brightness=std::clamp(number("Brightness",1),.6f,1.4f);settings.volume=std::clamp(number("Volume",.85f),0.f,1.f);
 for(int p=0;p<2;++p){auto prefix="P"+std::to_string(p+1);auto& v=players[p];v.mode=std::clamp(integer(prefix+"Mode",0),0,4);v.xindex=std::clamp(integer(prefix+"XInput",p),0,3);v.device=read(prefix+"Device","");v.keyboard=integer(prefix+"Keyboard",1)!=0;for(int a=0;a<10;++a){v.key[a]=std::clamp(integer(prefix+"Key"+std::to_string(a),v.key[a]),0,int(SDL_SCANCODE_COUNT)-1);auto b=read(prefix+"Bind"+std::to_string(a),"");if(!b.empty()){Binding binding{};if(sscanf_s(b.c_str(),"%d,%d,%d",&binding.type,&binding.index,&binding.value)==3&&binding.type>=0&&binding.type<=4&&binding.index>=0&&binding.index<65536)v.bind[a]=binding;}}}
 xinputModule=LoadLibraryW(L"xinput1_4.dll");if(!xinputModule)xinputModule=LoadLibraryW(L"xinput9_1_0.dll");if(xinputModule)getXInput=reinterpret_cast<decltype(getXInput)>(GetProcAddress(xinputModule,"XInputGetState"));refresh();
}
static bool borderlessApplied=false;static int previousX=0,previousY=0,previousW=1280,previousH=720;
void ApplyWindowMode(bool borderless){
 if(!win)return;
 if(borderless){
  SDL_Rect bounds{};auto display=SDL_GetDisplayForWindow(win);if(!display)display=SDL_GetPrimaryDisplay();if(!SDL_GetDisplayBounds(display,&bounds))return;
  if(!borderlessApplied){SDL_GetWindowPosition(win,&previousX,&previousY);SDL_GetWindowSize(win,&previousW,&previousH);}
  SDL_SetWindowFullscreen(win,false);SDL_SetWindowBordered(win,false);SDL_SetWindowPosition(win,bounds.x,bounds.y);SDL_SetWindowSize(win,bounds.w,bounds.h);borderlessApplied=true;
 }else if(borderlessApplied){SDL_SetWindowFullscreen(win,false);SDL_SetWindowBordered(win,true);SDL_SetWindowSize(win,previousW,previousH);SDL_SetWindowPosition(win,previousX,previousY);borderlessApplied=false;}
 SDL_SyncWindow(win);
}
static Device* chosen(int p){auto& v=players[p];if(v.mode==1||v.mode==3)return nullptr;if(v.mode==0)return size_t(p)<devices.size()?&devices[p]:nullptr;for(auto& d:devices)if(d.key==v.device)return &d;return nullptr;}
static void finishCapture(){capturePort=captureAction=-1;captureKind=0;dirty=true;blockUntil=SDL_GetTicks()+160;}
void Process(const SDL_Event& e){if(rffs::steam::OverlayVisible()){keys.fill(false);return;}
 if(e.type==SDL_EVENT_JOYSTICK_ADDED||e.type==SDL_EVENT_JOYSTICK_REMOVED||e.type==SDL_EVENT_GAMEPAD_ADDED||e.type==SDL_EVENT_GAMEPAD_REMOVED||e.type==SDL_EVENT_GAMEPAD_REMAPPED)refresh();
 if(e.type==SDL_EVENT_WINDOW_FOCUS_LOST){keys.fill(false);}
 if(e.type==SDL_EVENT_KEY_DOWN||e.type==SDL_EVENT_KEY_UP){if(e.key.scancode>0&&e.key.scancode<SDL_SCANCODE_COUNT)keys[e.key.scancode]=e.type==SDL_EVENT_KEY_DOWN;}
 if(e.type==SDL_EVENT_KEY_DOWN&&!e.key.repeat){
  if(capturePort>=0){if(e.key.scancode==SDL_SCANCODE_ESCAPE){capturePort=-1;captureKind=0;return;}if(captureKind==1&&e.key.scancode!=SDL_SCANCODE_F1&&e.key.scancode!=SDL_SCANCODE_F11){players[capturePort].key[captureAction]=e.key.scancode;finishCapture();return;}}
  if(e.key.scancode==SDL_SCANCODE_F4){settings.widescreen=!settings.widescreen;dirty=true;Notify(settings.widescreen?"Widescreen 16:9 ligado":"Modo original 4:3");return;}
  if(e.key.scancode==SDL_SCANCODE_F6){loadStateRequested=true;return;}
  if(e.key.scancode==SDL_SCANCODE_F3&&!visible){exUnlock=true;exFrame=0;exNoticeUntil=SDL_GetTicks()+5000;return;}
  if(e.key.scancode==SDL_SCANCODE_F2&&!visible){selectEX();return;}
  if(e.key.scancode==SDL_SCANCODE_F1){visible=!visible;capturePort=-1;blockUntil=SDL_GetTicks()+160;if(!visible)Save();return;}
  if(e.key.scancode==SDL_SCANCODE_ESCAPE){visible=!visible;capturePort=-1;blockUntil=SDL_GetTicks()+160;if(!visible)Save();return;}
  if(e.key.scancode==SDL_SCANCODE_F11){settings.fullscreen=!settings.fullscreen;ApplyWindowMode(settings.fullscreen);dirty=true;}
 }
 if(capturePort>=0&&captureKind==2){auto* d=chosen(capturePort);if(!d)return;auto& b=players[capturePort].bind[captureAction];
  if(players[capturePort].mode==4||!d->pad){
   if(e.type==SDL_EVENT_JOYSTICK_BUTTON_DOWN&&e.jbutton.which==d->id){b={1,e.jbutton.button,0};finishCapture();}
   else if(e.type==SDL_EVENT_JOYSTICK_HAT_MOTION&&e.jhat.which==d->id&&e.jhat.value){int value=e.jhat.value;value=value&SDL_HAT_UP?SDL_HAT_UP:(value&SDL_HAT_DOWN?SDL_HAT_DOWN:(value&SDL_HAT_LEFT?SDL_HAT_LEFT:SDL_HAT_RIGHT));b={2,e.jhat.hat,value};finishCapture();}
   else if(e.type==SDL_EVENT_JOYSTICK_AXIS_MOTION&&e.jaxis.which==d->id&&abs(e.jaxis.value)>24000){b={3,e.jaxis.axis,e.jaxis.value<0?-1:1};finishCapture();}
  }else if(e.type==SDL_EVENT_GAMEPAD_BUTTON_DOWN&&e.gbutton.which==d->id){b={0,e.gbutton.button,0};finishCapture();}
 }
}
void Update(){static Uint64 nextScan=0;if(SDL_GetTicks()>=nextScan){nextScan=SDL_GetTicks()+1000;int count=0;auto* ids=SDL_GetJoysticks(&count);std::vector<SDL_JoystickID> visibleIds;for(int i=0;i<count;++i){std::string k,n;if(!rffs::inputbridge::Active()||rffs::inputbridge::Identity(ids[i],k,n))visibleIds.push_back(ids[i]);}bool changed=visibleIds.size()!=devices.size();if(!changed)for(size_t i=0;i<visibleIds.size();++i)if(visibleIds[i]!=devices[i].id){changed=true;break;}SDL_free(ids);if(changed)refresh();}if(rffs::steam::OverlayVisible())keys.fill(false);if(capturePort>=0&&captureKind==2&&players[capturePort].mode==3&&getXInput){XINPUT_STATE state{};if(getXInput(players[capturePort].xindex,&state)==ERROR_SUCCESS){if(!state.Gamepad.wButtons)captureArmed=true;if(captureArmed&&state.Gamepad.wButtons){WORD b=state.Gamepad.wButtons;players[capturePort].bind[captureAction]={4,int(b&WORD(-short(b))),0};finishCapture();}}}
 if(dirty&&SDL_GetTicks()-lastSave>700)Save();}
static bool pressed(int p,int a){auto& v=players[p];if((v.mode==0||v.mode==1||v.keyboard)&&keys[v.key[a]])return true;
 if(v.mode==3){if(!getXInput)return false;XINPUT_STATE s{};if(getXInput(v.xindex,&s)!=ERROR_SUCCESS)return false;const int mask[]={XINPUT_GAMEPAD_DPAD_UP,XINPUT_GAMEPAD_DPAD_DOWN,XINPUT_GAMEPAD_DPAD_LEFT,XINPUT_GAMEPAD_DPAD_RIGHT,XINPUT_GAMEPAD_A,XINPUT_GAMEPAD_B,XINPUT_GAMEPAD_X,XINPUT_GAMEPAD_Y,XINPUT_GAMEPAD_START,XINPUT_GAMEPAD_BACK};auto& b=v.bind[a];int bit=b.type==4?b.index:mask[a];bool result=(s.Gamepad.wButtons&bit)!=0;if(a==0)result|=s.Gamepad.sThumbLY>16000;if(a==1)result|=s.Gamepad.sThumbLY<-16000;if(a==2)result|=s.Gamepad.sThumbLX<-16000;if(a==3)result|=s.Gamepad.sThumbLX>16000;return result;}
 auto* d=chosen(p);if(!d)return false;auto b=v.bind[a];if(v.mode==4||!d->pad){if(v.mode==0&&b.type==0){if(a<4)b={2,0,int(a==0?SDL_HAT_UP:(a==1?SDL_HAT_DOWN:(a==2?SDL_HAT_LEFT:SDL_HAT_RIGHT)))};else b={1,a<8?a-4:(a==8?7:6),0};}
 if(b.type==1&&b.index<SDL_GetNumJoystickButtons(d->stick))return SDL_GetJoystickButton(d->stick,b.index);
 if(b.type==2&&b.index<SDL_GetNumJoystickHats(d->stick))return (SDL_GetJoystickHat(d->stick,b.index)&b.value)!=0;
 if(b.type==3&&b.index<SDL_GetNumJoystickAxes(d->stick))return b.value*int(SDL_GetJoystickAxis(d->stick,b.index))>16000;
 return false;}
 bool result=b.type==0&&b.index<SDL_GAMEPAD_BUTTON_COUNT&&SDL_GetGamepadButton(d->pad,SDL_GamepadButton(b.index));if(a<4){int x=SDL_GetGamepadAxis(d->pad,SDL_GAMEPAD_AXIS_LEFTX),y=SDL_GetGamepadAxis(d->pad,SDL_GAMEPAD_AXIS_LEFTY);result|=(a==0&&y<-16000)||(a==1&&y>16000)||(a==2&&x<-16000)||(a==3&&x>16000);}return result;
}
// One command per accepted simulation frame; online commands pass through GGPO.
static uint16_t exMask(int f){
 const uint16_t start=1u<<3,b=1u<<8,c=1u<<1,a=1u<<0;
 if(exUnlock){
  const uint16_t l=1u<<6,r=1u<<7,u=1u<<4,d=1u<<5;
  const uint16_t directions[]={r,uint16_t(r|u),u,uint16_t(u|l),l,uint16_t(l|d),d,uint16_t(d|r),r};
  if(f<6)return 0;
  if(f<42)return directions[(f-6)/4];
  uint16_t mask=r|d;
  if(f>=46)mask|=a;
  if(f>=52)mask|=b;
  if(f>=58)mask|=c;
  return f<90?mask:0;
 }
 if(f<0||f>=150)return 0;
 uint16_t mask=start;
 if((f>=12&&f<18)||(f>=24&&f<30)||f>=60)mask|=b;
 if((f>=36&&f<42)||(f>=48&&f<54)||f>=126)mask|=c;
 if(f>=138&&f<144)mask|=a;
 return mask;
}
void AdvanceInput(){if(exFrame>=0&&++exFrame>=(exUnlock?96:150))exFrame=-1;}
void ShortcutHUD(){bool showNotice=SDL_GetTicks()<noticeUntil;if(!showNotice&&SDL_GetTicks()>=exNoticeUntil)return;ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x*.5f,76),ImGuiCond_Always,ImVec2(.5f,0));ImGui::SetNextWindowBgAlpha(.8f);ImGui::Begin("Atalho EX",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoSavedSettings);ImGui::TextUnformatted(showNotice?notice.c_str():exFrame>=0?"Executando comando EX...":"Sequência concluída; confira a seleção no jogo");ImGui::End();}
int Input(unsigned port,unsigned id){if(port>1||visible||rffs::steam::OverlayVisible()||SDL_GetTicks()<blockUntil||(!testMode&&SDL_GetKeyboardFocus()!=win))return 0;if(port==0&&exFrame>=0)return (exMask(exFrame)>>id)&1;static const int ids[]={4,5,6,7,0,8,1,9,3,2};for(int a=0;a<10;++a)if(unsigned(ids[a])==id){bool value=pressed(port,a);if(a<4&&pressed(port,a^1))value=false;return value;}return 0;}
static std::string bindingName(const Binding& b){if(b.type==0){const char* s=SDL_GetGamepadStringForButton(SDL_GamepadButton(b.index));return s?s:"Remapear";}if(b.type==1)return "Botão "+std::to_string(b.index+1);if(b.type==2)return "Direcional "+std::to_string(b.index+1)+" / "+std::to_string(b.value);if(b.type==3)return "Eixo "+std::to_string(b.index+1)+(b.value<0?" -":" +");return "XInput / "+std::to_string(b.index);}
static void heading(const char* title,const char* subtitle){ImGui::SetWindowFontScale(1.35f);ImGui::TextUnformatted(title);ImGui::SetWindowFontScale(1);ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(.55f,.64f,.75f,1));ImGui::TextWrapped("%s",subtitle);ImGui::PopStyleColor();ImGui::Spacing();ImGui::Separator();ImGui::Spacing();}
void SetTab(int t){tab=std::clamp(t,0,3);}
void Draw(bool gpu){if(!visible)return;auto& io=ImGui::GetIO();ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x*.5f,io.DisplaySize.y*.5f),ImGuiCond_Appearing,ImVec2(.5f,.5f));ImGui::SetNextWindowSize(ImVec2(std::min(960.f,io.DisplaySize.x-32),std::min(600.f,io.DisplaySize.y-32)),ImGuiCond_Appearing);
 ImGui::Begin("Preferências • Real Bout Special",&visible,ImGuiWindowFlags_NoCollapse);ImGui::BeginChild("navigation",ImVec2(174,0),ImGuiChildFlags_Borders);ImGui::Spacing();ImGui::SetWindowFontScale(1.4f);ImGui::TextUnformatted("REAL BOUT");ImGui::SetWindowFontScale(1);ImGui::TextColored(ImVec4(.3f,.85f,1,1),"SPECIAL");ImGui::Spacing();ImGui::Separator();ImGui::Spacing();const char* tabs[]={"Jogo","Controles","Imagem","Online"};for(int i=0;i<4;++i){if(ImGui::Selectable(tabs[i],tab==i,0,ImVec2(0,40)))tab=i;}ImGui::SetCursorPosY(std::max(ImGui::GetCursorPosY(),ImGui::GetWindowHeight()-100));ImGui::TextDisabled("F1 / Esc: voltar");ImGui::TextDisabled("Salvo automaticamente");ImGui::EndChild();ImGui::SameLine();ImGui::BeginChild("content",ImVec2(0,0));
 if(tab==0){heading("Do seu jeito","Ajustes do jogo, inicialização e áudio.");dirty|=ImGui::Checkbox("Entrar direto no jogo",&settings.directBoot);ImGui::TextDisabled("Pula a tela da BIOS na próxima abertura.");ImGui::Spacing();ImGui::TextUnformatted("F6: voltar ao estado inicial");ImGui::TextUnformatted("Região da BIOS: Japão");ImGui::TextDisabled("Região japonesa aplicada automaticamente.");ImGui::Spacing();float percent=settings.volume*100;if(ImGui::SliderFloat("Volume",&percent,0,100,"%.0f%%",ImGuiSliderFlags_AlwaysClamp)){settings.volume=percent/100;dirty=true;}if(ImGui::Checkbox("Tela inteira sem bordas (F11)",&settings.fullscreen)){ApplyWindowMode(settings.fullscreen);dirty=true;}ImGui::Spacing();ImGui::Separator();ImGui::Spacing();if(ImGui::Button("Voltar ao jogo",ImVec2(190,38))){visible=false;blockUntil=SDL_GetTicks()+160;Save();}ImGui::SameLine();ImGui::BeginDisabled(rffs::net::Locked());if(ImGui::Button("Reiniciar jogo",ImVec2(160,38)))ImGui::OpenPopup("Reiniciar?");ImGui::EndDisabled();if(ImGui::BeginPopupModal("Reiniciar?",nullptr,ImGuiWindowFlags_AlwaysAutoResize)){ImGui::TextUnformatted("A partida atual será encerrada.");if(ImGui::Button("Reiniciar",ImVec2(140,34))){Save();restartRequested=true;visible=false;ImGui::CloseCurrentPopup();}ImGui::SameLine();if(ImGui::Button("Cancelar",ImVec2(140,34)))ImGui::CloseCurrentPopup();ImGui::EndPopup();}ImGui::Spacing();if(ImGui::Button("Sair",ImVec2(150,32)))quitRequested=true;}
 else if(tab==1){heading("Controles","Escolha um dispositivo por jogador e personalize os botões.");ImGui::TextWrapped("F3: na tela de ranking, executa a liberação dos secretos. Aguarde a voz de Geese. EX: destaque Andy, Billy, Mary ou Tung na seleção e pressione F2. Cada jogador usa F2 no seu PC durante o online.");if(ImGui::Button("Atualizar controles")){SDL_UpdateJoysticks();refresh();}ImGui::SameLine();ImGui::TextDisabled("%d dispositivo(s) detectado(s)",int(devices.size()));static int player=0;ImGui::RadioButton("Jogador 1",&player,0);ImGui::SameLine();ImGui::RadioButton("Jogador 2",&player,1);auto& p=players[player];std::string label=p.mode==0?"Automático • controle + teclado":p.mode==1?"Somente teclado":p.mode==3?"XInput • controle "+std::to_string(p.xindex+1):"Dispositivo desconectado";auto* d=chosen(player);if(d&&p.mode!=0)label=(p.mode==4?"Arcade / joystick • ":"SDL • ")+d->name;
 if(ImGui::BeginCombo("Dispositivo",label.c_str())){if(ImGui::Selectable("Automático • controle + teclado",p.mode==0)){p.mode=0;dirty=true;}if(ImGui::Selectable("Somente teclado",p.mode==1)){p.mode=1;dirty=true;}for(auto& dev:devices){ImGui::PushID(int(dev.id));std::string text=(dev.pad?"SDL • ":"Arcade / joystick • ")+dev.name;if(ImGui::Selectable(text.c_str(),p.device==dev.key&&p.mode==(dev.pad?2:4))){p.mode=dev.pad?2:4;p.device=dev.key;if(dev.pad){const int buttons[]={SDL_GAMEPAD_BUTTON_DPAD_UP,SDL_GAMEPAD_BUTTON_DPAD_DOWN,SDL_GAMEPAD_BUTTON_DPAD_LEFT,SDL_GAMEPAD_BUTTON_DPAD_RIGHT,SDL_GAMEPAD_BUTTON_SOUTH,SDL_GAMEPAD_BUTTON_EAST,SDL_GAMEPAD_BUTTON_WEST,SDL_GAMEPAD_BUTTON_NORTH,SDL_GAMEPAD_BUTTON_START,SDL_GAMEPAD_BUTTON_BACK};for(int a=0;a<10;++a)p.bind[a]={0,buttons[a],0};}if(!dev.pad){for(int a=0;a<10;++a)p.bind[a]=a<4?Binding{2,0,int(a==0?SDL_HAT_UP:(a==1?SDL_HAT_DOWN:(a==2?SDL_HAT_LEFT:SDL_HAT_RIGHT)))}:Binding{1,a<8?a-4:(a==8?7:6),0};}dirty=true;}if(dev.pad){std::string raw="Joystick bruto • "+dev.name;if(ImGui::Selectable(raw.c_str(),p.device==dev.key&&p.mode==4)){p.mode=4;p.device=dev.key;for(int a=0;a<10;++a)p.bind[a]=a<4?Binding{2,0,int(a==0?SDL_HAT_UP:(a==1?SDL_HAT_DOWN:(a==2?SDL_HAT_LEFT:SDL_HAT_RIGHT)))}:Binding{1,a<8?a-4:(a==8?7:6),0};dirty=true;}}ImGui::PopID();}for(int i=0;i<4;++i){std::string text="XInput • controle "+std::to_string(i+1);if(ImGui::Selectable(text.c_str(),p.mode==3&&p.xindex==i)){p.mode=3;p.xindex=i;dirty=true;}}ImGui::EndCombo();}
 if(p.mode!=0&&p.mode!=1)dirty|=ImGui::Checkbox("Também permitir o teclado",&p.keyboard);
 ImGui::TextDisabled("Para remapear, clique no campo e pressione uma tecla ou botão.");
 if(ImGui::BeginTable("bindings",3,ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH)){ImGui::TableSetupColumn("Ação");ImGui::TableSetupColumn("Teclado");ImGui::TableSetupColumn("Controle / arcade");ImGui::TableHeadersRow();for(int a=0;a<10;++a){ImGui::PushID(a);ImGui::TableNextRow();ImGui::TableSetColumnIndex(0);ImGui::TextUnformatted(actionNames[a]);ImGui::TableSetColumnIndex(1);const char* key=SDL_GetScancodeName(SDL_Scancode(p.key[a]));std::string text=capturePort==player&&captureAction==a&&captureKind==1?"Pressione a tecla...":(key&&*key?key:"Definir tecla");if(ImGui::Button((text+"##key").c_str(),ImVec2(-1,0))){capturePort=player;captureAction=a;captureKind=1;}ImGui::TableSetColumnIndex(2);text=capturePort==player&&captureAction==a&&captureKind==2?"Pressione o botão...":bindingName(p.bind[a]);if(ImGui::Button((text+"##pad").c_str(),ImVec2(-1,0))){capturePort=player;captureAction=a;captureKind=2;captureArmed=false;}ImGui::PopID();}ImGui::EndTable();}
 if(capturePort>=0)ImGui::TextColored(ImVec4(.35f,.85f,1,1),"Aguardando entrada. Esc cancela.");
 ImGui::Spacing();ImGui::TextWrapped("DualSense e DualShock 4: USB ou Bluetooth pelo SDL. DualShock 3 depende de um driver compatível do Windows, como DsHidMini ou Sixaxis. Arcades genéricos podem ser mapeados como joystick bruto.");
 if(ImGui::Button("Restaurar mapeamentos")){defaults();dirty=true;}}
 else if(tab==2){heading("Imagem","Escolha o visual; a velocidade e os comandos do jogo não mudam.");if(ImGui::Checkbox("Widescreen experimental 16:9 (F4)",&settings.widescreen)){dirty=true;Notify(settings.widescreen?"Widescreen 16:9 ligado":"Modo original 4:3");}ImGui::TextDisabled("Desligado: imagem original 4:3. Preferência salva automaticamente.");const char* names[]={"Original • pixels nítidos","Suave • interpolação","Pixel suave • bordas 2x","CRT • linhas e máscara RGB"};dirty|=ImGui::Combo(gpu?"Shader":"Filtro",&settings.filter,names,4);if(settings.filter==1||settings.filter==2)dirty|=ImGui::SliderFloat("Suavização",&settings.smoothing,0,1,"%.2f");dirty|=ImGui::SliderFloat("Brilho",&settings.brightness,.6f,1.4f,"%.2f");ImGui::Spacing();ImGui::TextWrapped("Pixel suave arredonda os degraus da pixel art sem alterar os arquivos do jogo. Original preserva cada pixel; CRT simula uma tela de tubo.");if(!gpu)ImGui::TextColored(ImVec4(1,.75f,.35f,1),"Vídeo Direct3D / compatível: filtros disponíveis acima.");if(ImGui::Button("Restaurar imagem")){settings.filter=2;settings.smoothing=.8f;settings.brightness=1;dirty=true;}}
 else {heading("Online","Versus com GGPO e convites Steam.");rffs::net::Draw();}
 ImGui::EndChild();ImGui::End();if(!visible){blockUntil=SDL_GetTicks()+160;Save();}
}
void Shutdown(){Save();for(auto& d:devices){if(d.pad)SDL_CloseGamepad(d.pad);else if(d.stick)SDL_CloseJoystick(d.stick);}devices.clear();if(xinputModule)FreeLibrary(xinputModule);}
static XINPUT_STATE mockXInput{};
static DWORD WINAPI testXInput(DWORD index,XINPUT_STATE* state){if(index!=2)return ERROR_DEVICE_NOT_CONNECTED;*state=mockXInput;return ERROR_SUCCESS;}
bool InputSelfTest(){
 auto backup=players;bool previousVisible=visible;visible=false;blockUntil=0;players[0].mode=1;players[0].key[4]=SDL_SCANCODE_Z;
 SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.scancode=SDL_SCANCODE_Z;Process(e);bool ok=Input(0,0)==1;e.type=SDL_EVENT_KEY_UP;Process(e);ok&=Input(0,0)==0;
 e.type=SDL_EVENT_KEY_DOWN;e.key.scancode=SDL_SCANCODE_LEFT;Process(e);e.key.scancode=SDL_SCANCODE_RIGHT;Process(e);ok&=Input(0,6)==0&&Input(0,7)==0;keys.fill(false);
 visible=true;e.type=SDL_EVENT_KEY_DOWN;e.key.scancode=SDL_SCANCODE_Z;Process(e);ok&=Input(0,0)==0;keys.fill(false);visible=false;
 printf("INPUT keyboard/menu=%d\n",ok);
 SDL_VirtualJoystickDesc desc;SDL_INIT_INTERFACE(&desc);desc.type=SDL_JOYSTICK_TYPE_GAMEPAD;desc.nbuttons=SDL_GAMEPAD_BUTTON_COUNT;desc.naxes=SDL_GAMEPAD_AXIS_COUNT;desc.button_mask=(1u<<SDL_GAMEPAD_BUTTON_COUNT)-1;desc.axis_mask=(1u<<SDL_GAMEPAD_AXIS_COUNT)-1;desc.name="RFFS SDL test";
 auto padID=SDL_AttachVirtualJoystick(&desc);desc.type=SDL_JOYSTICK_TYPE_ARCADE_STICK;desc.name="RFFS Arcade test";desc.nhats=1;auto rawID=SDL_AttachVirtualJoystick(&desc);refresh();
 SDL_Joystick* padStick=nullptr;SDL_Joystick* rawStick=nullptr;
 for(auto& d:devices){if(d.id==padID){padStick=d.stick;players[0].mode=2;players[0].device=d.key;players[0].keyboard=false;players[0].bind[4]={0,SDL_GAMEPAD_BUTTON_SOUTH,0};}if(d.id==rawID){rawStick=d.stick;players[1].mode=4;players[1].device=d.key;players[1].keyboard=false;players[1].bind[6]={1,2,0};players[1].bind[2]={2,0,SDL_HAT_LEFT};players[1].bind[3]={3,0,1};}}
 bool padOK=padStick!=nullptr,rawOK=rawStick!=nullptr;
 if(padStick){SDL_SetJoystickVirtualButton(padStick,SDL_GAMEPAD_BUTTON_SOUTH,true);SDL_UpdateJoysticks();padOK&=Input(0,0)==1&&Input(1,0)==0;SDL_SetJoystickVirtualButton(padStick,SDL_GAMEPAD_BUTTON_SOUTH,false);SDL_UpdateJoysticks();padOK&=Input(0,0)==0;SDL_SetJoystickVirtualAxis(padStick,SDL_GAMEPAD_AXIS_LEFTY,-24000);SDL_UpdateJoysticks();padOK&=Input(0,4)==1;SDL_SetJoystickVirtualAxis(padStick,SDL_GAMEPAD_AXIS_LEFTY,0);SDL_UpdateJoysticks();}
 if(rawStick){SDL_SetJoystickVirtualButton(rawStick,2,true);SDL_SetJoystickVirtualHat(rawStick,0,SDL_HAT_LEFT);SDL_UpdateJoysticks();rawOK&=Input(1,1)==1&&Input(1,6)==1&&Input(0,1)==0;SDL_SetJoystickVirtualButton(rawStick,2,false);SDL_SetJoystickVirtualHat(rawStick,0,0);SDL_SetJoystickVirtualAxis(rawStick,0,24000);SDL_UpdateJoysticks();rawOK&=Input(1,1)==0&&Input(1,7)==1;SDL_SetJoystickVirtualAxis(rawStick,0,0);SDL_UpdateJoysticks();}
 printf("INPUT virtual SDL=%d raw arcade=%d\n",padOK,rawOK);ok&=padOK&&rawOK;
 if(padID)SDL_DetachVirtualJoystick(padID);if(rawID)SDL_DetachVirtualJoystick(rawID);refresh();ok&=chosen(0)==nullptr&&chosen(1)==nullptr;
 auto realXInput=getXInput;getXInput=testXInput;players[0].mode=3;players[0].xindex=2;players[0].keyboard=false;players[0].bind[4]={4,XINPUT_GAMEPAD_A,0};mockXInput={};mockXInput.Gamepad.wButtons=XINPUT_GAMEPAD_A;bool xOK=Input(0,0)==1;mockXInput={};mockXInput.Gamepad.sThumbLX=24000;xOK&=Input(0,0)==0&&Input(0,7)==1;players[0].xindex=1;xOK&=Input(0,7)==0;getXInput=realXInput;ok&=xOK;
 players[0].key[4]=SDL_SCANCODE_Q;Save();bool saved=integer("P1Key4",0)==SDL_SCANCODE_Q&&integer("P1Mode",0)==3;ok&=saved;printf("INPUT XInput path=%d config save/read=%d hotplug=1\n",xOK,saved);
 keys.fill(false);players=backup;Save();visible=previousVisible;blockUntil=0;return ok;
}
}