#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#define SDL_MAIN_HANDLED
#include "rffs_input_bridge.h"
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <map>
#include <vector>
namespace rffs::inputbridge {
namespace {
constexpr int maxDevices=16,maxAxes=16,maxButtons=64,maxHats=4;
struct State {Uint32 id=0;int axes=0,buttons=0,hats=0;bool pad=false;char name[256]{},key[256]{},mapping[4096]{};Sint16 axis[maxAxes]{};bool button[maxButtons]{};Uint8 hat[maxHats]{};};
struct Shared {Uint32 magic=0;bool ready=false,quit=false;int testStage=0,count=0;State devices[maxDevices]{};};
struct Mirror {Uint32 remote=0;SDL_JoystickID id=0;SDL_Joystick* stick=nullptr;std::string name,key;};
HANDLE memory=nullptr,mutex=nullptr,child=nullptr;Shared* shared=nullptr;std::vector<Mirror> mirrors;bool active=false;
std::wstring mapName(DWORD pid){return L"Local\\RFFS-Input-"+std::to_wstring(pid);}
bool lock(){return mutex&&WaitForSingleObject(mutex,20)==WAIT_OBJECT_0;}
void unlock(){ReleaseMutex(mutex);}
void closeMirror(Mirror& m){if(m.stick)SDL_CloseJoystick(m.stick);if(m.id)SDL_DetachVirtualJoystick(m.id);}
}
bool Active(){return active;}
bool Identity(SDL_JoystickID id,std::string& key,std::string& name){for(auto& m:mirrors)if(m.id==id){key=m.key;name=m.name;return true;}return false;}
bool Start(const std::wstring& exe){
 auto name=mapName(GetCurrentProcessId());memory=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(Shared),name.c_str());if(!memory)return false;
 shared=static_cast<Shared*>(MapViewOfFile(memory,FILE_MAP_ALL_ACCESS,0,0,sizeof(Shared)));mutex=CreateMutexW(nullptr,FALSE,(name+L"-lock").c_str());if(!shared||!mutex){Stop();return false;}*shared=Shared{};shared->magic=0x52464931;
 std::wstring command=L"\""+exe+L"\" --input-service "+std::to_wstring(GetCurrentProcessId());STARTUPINFOW si{};si.cb=sizeof(si);si.dwFlags=STARTF_USESHOWWINDOW;si.wShowWindow=SW_HIDE;PROCESS_INFORMATION pi{};
 if(!CreateProcessW(exe.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&si,&pi)){Stop();return false;}child=pi.hProcess;CloseHandle(pi.hThread);
 for(int i=0;i<200;++i){if(lock()){active=shared->ready;unlock();}if(active)return true;if(WaitForSingleObject(child,10)==WAIT_OBJECT_0)break;}Stop();return false;
}
void Stop(){active=false;if(shared&&lock()){shared->quit=true;unlock();}if(child){WaitForSingleObject(child,500);CloseHandle(child);child=nullptr;}for(auto& m:mirrors)closeMirror(m);mirrors.clear();if(shared){UnmapViewOfFile(shared);shared=nullptr;}if(memory){CloseHandle(memory);memory=nullptr;}if(mutex){CloseHandle(mutex);mutex=nullptr;}}
void Update(){if(!active||!shared)return;if(WaitForSingleObject(child,0)==WAIT_OBJECT_0){active=false;return;}Shared snapshot;if(!lock())return;snapshot=*shared;unlock();snapshot.count=std::clamp(snapshot.count,0,maxDevices);
 for(auto it=mirrors.begin();it!=mirrors.end();){bool found=false;for(int i=0;i<snapshot.count;++i)found|=snapshot.devices[i].id==it->remote;if(!found){closeMirror(*it);it=mirrors.erase(it);}else ++it;}
 for(int i=0;i<snapshot.count;++i){const auto& d=snapshot.devices[i];auto it=std::find_if(mirrors.begin(),mirrors.end(),[&](auto& m){return m.remote==d.id;});
  if(it==mirrors.end()){SDL_VirtualJoystickDesc desc;SDL_INIT_INTERFACE(&desc);desc.type=d.pad?SDL_JOYSTICK_TYPE_GAMEPAD:SDL_JOYSTICK_TYPE_ARCADE_STICK;desc.naxes=d.axes;desc.nbuttons=d.buttons;desc.nhats=d.hats;desc.name=d.name;auto id=SDL_AttachVirtualJoystick(&desc);if(!id)continue;
   if(d.pad&&d.mapping[0]){std::string mapping=d.mapping;auto comma=mapping.find(',');if(comma!=std::string::npos){char guid[33];SDL_GUIDToString(SDL_GetJoystickGUIDForID(id),guid,sizeof(guid));mapping.replace(0,comma,guid);SDL_SetGamepadMapping(id,mapping.c_str());}}
   auto* stick=SDL_OpenJoystick(id);if(!stick){SDL_DetachVirtualJoystick(id);continue;}mirrors.push_back({d.id,id,stick,d.name,d.key});it=mirrors.end()-1;printf("INPUT bridge connected: %s\n",d.name);
  }
  for(int a=0;a<d.axes;++a)SDL_SetJoystickVirtualAxis(it->stick,a,d.axis[a]);for(int b=0;b<d.buttons;++b)SDL_SetJoystickVirtualButton(it->stick,b,d.button[b]);for(int h=0;h<d.hats;++h)SDL_SetJoystickVirtualHat(it->stick,h,d.hat[h]);
 }
 SDL_UpdateJoysticks();
}
int Service(unsigned long parent){
 auto name=mapName(parent);HANDLE map=OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,name.c_str());if(!map)return 1;auto* data=static_cast<Shared*>(MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(Shared)));HANDLE guard=OpenMutexW(SYNCHRONIZE|MUTEX_MODIFY_STATE,FALSE,(name+L"-lock").c_str());HANDLE owner=OpenProcess(SYNCHRONIZE,FALSE,parent);if(!data||!guard||!owner)return 1;
 SDL_SetMainReady();SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI,"1");SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4,"1");SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5,"1");SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3,"1");SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3_SIXAXIS_DRIVER,"1");
 if(!SDL_Init(SDL_INIT_GAMEPAD|SDL_INIT_JOYSTICK))return 1;wchar_t executable[32768];GetModuleFileNameW(nullptr,executable,32768);auto database=(std::filesystem::path(executable).parent_path()/L"rffs-runtime/gamecontrollerdb.txt").u8string();SDL_AddGamepadMappingsFromFile(reinterpret_cast<const char*>(database.c_str()));
 struct Device {SDL_JoystickID id;SDL_Gamepad* pad;SDL_Joystick* stick;State info;};std::vector<Device> devices;Uint64 scanAt=0;SDL_JoystickID testID=0;SDL_Joystick* testStick=nullptr;int previousStage=0;
 while(WaitForSingleObject(owner,0)==WAIT_TIMEOUT){bool quit=false;int stage=0;if(WaitForSingleObject(guard,100)==WAIT_OBJECT_0){quit=data->quit;stage=data->testStage;ReleaseMutex(guard);}if(quit)break;
  if(stage!=previousStage){if(testStick){SDL_CloseJoystick(testStick);testStick=nullptr;}if(testID){SDL_DetachVirtualJoystick(testID);testID=0;}if(stage==1||stage==3||stage==5){SDL_VirtualJoystickDesc desc;SDL_INIT_INTERFACE(&desc);desc.type=SDL_JOYSTICK_TYPE_GAMEPAD;desc.nbuttons=SDL_GAMEPAD_BUTTON_COUNT;desc.naxes=SDL_GAMEPAD_AXIS_COUNT;desc.button_mask=(1u<<SDL_GAMEPAD_BUTTON_COUNT)-1;desc.axis_mask=(1u<<SDL_GAMEPAD_AXIS_COUNT)-1;if(stage==5){desc.type=SDL_JOYSTICK_TYPE_ARCADE_STICK;desc.nbuttons=8;desc.naxes=2;desc.nhats=1;desc.button_mask=desc.axis_mask=0;}desc.name="RFFS hotplug test";testID=SDL_AttachVirtualJoystick(&desc);testStick=SDL_OpenJoystick(testID);if(testStick){SDL_SetJoystickVirtualButton(testStick,stage==5?2:SDL_GAMEPAD_BUTTON_SOUTH,true);if(stage==5){SDL_SetJoystickVirtualHat(testStick,0,SDL_HAT_LEFT);SDL_SetJoystickVirtualAxis(testStick,0,24000);}}}previousStage=stage;scanAt=0;}
  SDL_UpdateJoysticks();if(SDL_GetTicks()>=scanAt){scanAt=SDL_GetTicks()+250;int count=0;auto* ids=SDL_GetJoysticks(&count);std::vector<SDL_JoystickID> wanted;for(int i=0;i<count&&wanted.size()<maxDevices;++i)wanted.push_back(ids[i]);SDL_free(ids);
   bool changed=wanted.size()!=devices.size();if(!changed)for(size_t i=0;i<wanted.size();++i)changed|=wanted[i]!=devices[i].id;
   if(changed){for(auto& d:devices){if(d.pad)SDL_CloseGamepad(d.pad);else SDL_CloseJoystick(d.stick);}devices.clear();std::map<std::string,int> ordinals;
    for(auto id:wanted){Device d{};d.id=id;if(SDL_IsGamepad(id))d.pad=SDL_OpenGamepad(id);d.stick=d.pad?SDL_GetGamepadJoystick(d.pad):SDL_OpenJoystick(id);if(!d.stick)continue;auto& info=d.info;info.id=id;info.pad=d.pad!=nullptr;info.axes=std::clamp(SDL_GetNumJoystickAxes(d.stick),0,maxAxes);info.buttons=std::clamp(SDL_GetNumJoystickButtons(d.stick),0,maxButtons);info.hats=std::clamp(SDL_GetNumJoystickHats(d.stick),0,maxHats);const char* title=SDL_GetJoystickName(d.stick);snprintf(info.name,sizeof(info.name),"%s",title?title:"Controle");char guid[33];SDL_GUIDToString(SDL_GetJoystickGUID(d.stick),guid,sizeof(guid));std::string key=guid;const char* serial=SDL_GetJoystickSerial(d.stick);if(serial&&*serial)key+="/"+std::string(serial);key+="#"+std::to_string(ordinals[key]++);snprintf(info.key,sizeof(info.key),"%s",key.c_str());if(d.pad){char* mapping=SDL_GetGamepadMapping(d.pad);if(mapping){snprintf(info.mapping,sizeof(info.mapping),"%s",mapping);SDL_free(mapping);}}devices.push_back(d);}
   }
  }
  Shared snapshot;snapshot.count=int(devices.size());for(size_t i=0;i<devices.size();++i){auto& d=devices[i];snapshot.devices[i]=d.info;auto& info=snapshot.devices[i];for(int a=0;a<info.axes;++a)info.axis[a]=SDL_GetJoystickAxis(d.stick,a);for(int b=0;b<info.buttons;++b)info.button[b]=SDL_GetJoystickButton(d.stick,b);for(int h=0;h<info.hats;++h)info.hat[h]=SDL_GetJoystickHat(d.stick,h);}
  if(WaitForSingleObject(guard,100)==WAIT_OBJECT_0){data->count=snapshot.count;std::copy(std::begin(snapshot.devices),std::end(snapshot.devices),std::begin(data->devices));data->ready=true;ReleaseMutex(guard);}SDL_Delay(4);
 }
 for(auto& d:devices){if(d.pad)SDL_CloseGamepad(d.pad);else SDL_CloseJoystick(d.stick);}if(testStick)SDL_CloseJoystick(testStick);if(testID)SDL_DetachVirtualJoystick(testID);SDL_Quit();CloseHandle(owner);CloseHandle(guard);UnmapViewOfFile(data);CloseHandle(map);return 0;
}
bool SelfTest(){if(!active)return false;SDL_JoystickID first=0;for(int stage:{1,2,3,4,5}){if(lock()){shared->testStage=stage;unlock();}bool good=false;for(int i=0;i<200&&!good;++i){Update();Mirror* found=nullptr;for(auto& m:mirrors)if(m.name=="RFFS hotplug test")found=&m;if(stage==2||stage==4)good=!found;else if(stage==5&&found)good=!SDL_IsGamepad(found->id)&&SDL_GetJoystickButton(found->stick,2)&&SDL_GetJoystickHat(found->stick,0)==SDL_HAT_LEFT&&SDL_GetJoystickAxis(found->stick,0)==24000;else if(found&&SDL_IsGamepad(found->id)){auto* pad=SDL_OpenGamepad(found->id);good=pad&&SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_SOUTH);if(pad)SDL_CloseGamepad(pad);if(stage==1)first=found->id;if(stage==3)good=good&&found->id!=first;}if(!good)SDL_Delay(10);}if(!good)return false;}if(lock()){shared->testStage=6;unlock();}printf("HOTPLUG_BRIDGE_PASS late attach, mapped button, disconnect, reconnect, raw arcade button/hat/axis\n");return true;}
}