#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include "FBNeo/src/burner/libretro/libretro-common/include/libretro.h"
#include <cstdio>
#include <cstdarg>
#include <map>
#include <string>
#include <filesystem>
#include <vector>
#include <array>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <algorithm>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_sdlrenderer3.h"
#include "rffs_gl.h"
#include "rffs_menu.h"
#include "rffs_input_bridge.h"
#include "rffs_net.h"
#include "rffs_steam.h"
#include <fstream>
namespace fs=std::filesystem;
static char directory[1024];static std::map<std::string,std::string> options;
static int frames=0,errors=0;static unsigned pixelFormat=RETRO_PIXEL_FORMAT_0RGB1555;
static SDL_Window* window=nullptr;static SDL_Renderer* renderer=nullptr;static SDL_Texture* texture=nullptr;static SDL_AudioStream* audioStream=nullptr;
static int* nativeScreenWidth=nullptr;
static unsigned textureW=0,textureH=0,frameW=0,frameH=0;
static bool hotplugTest=false,widescreenTest=false,preparationTest=false,startInputTest=false,direct3DTest=false,overlayTest=false,menuTest=false,steamCheck=false,rollbackTest=false,netExecuting=false,replaying=false;static uint16_t netInputs[2]{};
static bool testing=false,gpuTest=false,gpu=false,booting=false,quit=false;static unsigned bootBegin=0;
static std::vector<uint32_t> pixels,processed;static std::vector<int16_t> adjustedAudio;
static uint64_t audioFrames=0,nonzeroAudio=0;static uint8_t* programRam=nullptr;static size_t programRamSize=0;
static unsigned (*guestPC)(int)=nullptr;static fs::path runtime;
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(std::string(message)+": "+SDL_GetError());}
static UINT16 word(size_t a){return a+2<=programRamSize?*reinterpret_cast<const uint16_t*>(programRam+a):0;}
static bool gameCodeReady(){return programRam&&programRamSize>=0x110000&&word(0x108)==0x0223&&word(0x50b4)==0x3d41&&word(0x50b6)==2;}
static bool gameRunning(){return gameCodeReady()&&(!guestPC||guestPC(0)<0x200000);}

static void logcb(retro_log_level l,const char* f,...){if(l>=RETRO_LOG_ERROR)++errors;va_list a;va_start(a,f);vfprintf(stdout,f,a);va_end(a);}
static bool env(unsigned command,void* p){switch(command){
 case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:{auto* m=static_cast<retro_memory_map*>(p);for(unsigned i=0;i<m->num_descriptors;++i){const auto& d=m->descriptors[i];if(d.start==0&&d.len>=0x200000){programRam=static_cast<uint8_t*>(const_cast<void*>(d.ptr))+d.offset;programRamSize=d.len;}}return true;}
 case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:*static_cast<const char**>(p)=directory;return true;
 case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:static_cast<retro_log_callback*>(p)->log=logcb;return true;
 case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:*static_cast<unsigned*>(p)=0;return true;
 case RETRO_ENVIRONMENT_GET_LANGUAGE:*static_cast<unsigned*>(p)=RETRO_LANGUAGE_ENGLISH;return true;
 case RETRO_ENVIRONMENT_GET_CAN_DUPE:*static_cast<bool*>(p)=true;return true;
 case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:*static_cast<bool*>(p)=false;return true;
 case RETRO_ENVIRONMENT_SET_VARIABLES:for(auto* v=static_cast<retro_variable*>(p);v->key;++v){std::string value=v->value;auto i=value.find(';');if(i!=value.npos){value=value.substr(i+2);options[v->key]=value.substr(0,value.find('|'));}}return true;
 case RETRO_ENVIRONMENT_GET_VARIABLE:{auto* v=static_cast<retro_variable*>(p);if(!strcmp(v->key,"fbneo-diagnostic-input")){v->value="None";return true;}if(!strcmp(v->key,"fbneo-dipswitch-neocdz-Region")){v->value="Japan";return true;}auto i=options.find(v->key);v->value=i==options.end()?nullptr:i->second.c_str();return v->value!=nullptr;}
 case RETRO_ENVIRONMENT_SET_MESSAGE:puts(static_cast<retro_message*>(p)->msg);return true;
 case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:pixelFormat=*static_cast<unsigned*>(p);return pixelFormat<=2;
 case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME:case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO:case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:case RETRO_ENVIRONMENT_SET_GEOMETRY:return true;
 default:return false;
}}
static uint64_t stateFingerprint(const std::vector<uint8_t>& bytes){uint64_t h=14695981039346656037ull;for(auto v:bytes){h^=v;h*=1099511628211ull;}return h;}
static void writeManualState(const fs::path& folder,uint64_t compatibility,const std::vector<uint8_t>& state){
 auto target=folder/L"estado-manual.rffsstate";auto temporary=folder/L"estado-manual.rffsstate.tmp";
 std::ofstream out(temporary,std::ios::binary|std::ios::trunc);const char magic[8]={'R','F','F','S','S','T','0','2'};uint64_t size=state.size(),checksum=stateFingerprint(state);
 out.write(magic,8);out.write(reinterpret_cast<const char*>(&compatibility),8);out.write(reinterpret_cast<const char*>(&size),8);out.write(reinterpret_cast<const char*>(&checksum),8);out.write(reinterpret_cast<const char*>(state.data()),std::streamsize(state.size()));out.flush();bool ok=bool(out);out.close();
 if(!ok||out.fail()||!MoveFileExW(temporary.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Não foi possível salvar o estado manual.");
}
static std::vector<uint8_t> readManualState(const fs::path& folder,uint64_t compatibility,size_t expected,const wchar_t* filename=L"estado-manual.rffsstate"){
 auto file=folder/filename;if(!fs::exists(file))return {};
 std::ifstream in(file,std::ios::binary);char magic[8]{};uint64_t key=0,size=0,checksum=0;
 in.read(magic,8);in.read(reinterpret_cast<char*>(&key),8);in.read(reinterpret_cast<char*>(&size),8);in.read(reinterpret_cast<char*>(&checksum),8);
 if(!in||memcmp(magic,"RFFSST02",8)||key!=compatibility||size!=expected||size>32*1024*1024||fs::file_size(file)!=size+32)throw std::runtime_error("Estado manual incompatível; iniciando normalmente.");
 std::vector<uint8_t> bytes(size);in.read(reinterpret_cast<char*>(bytes.data()),std::streamsize(size));if(!in||stateFingerprint(bytes)!=checksum)throw std::runtime_error("Estado manual danificado; iniciando normalmente.");return bytes;
}
static void render();
static void video(const void* data,unsigned w,unsigned h,size_t pitch){++frames;if(!data)return;if(nativeScreenWidth&&*nativeScreenWidth==320)w=320;frameW=w;frameH=h;pixels.resize(w*h);
 for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){const auto* row=static_cast<const uint8_t*>(data)+y*pitch;uint32_t color;
  if(pixelFormat==RETRO_PIXEL_FORMAT_XRGB8888)color=reinterpret_cast<const uint32_t*>(row)[x];else{auto v=reinterpret_cast<const uint16_t*>(row)[x];unsigned r,g,b;if(pixelFormat==RETRO_PIXEL_FORMAT_RGB565){r=((v>>11)&31)*255/31;g=((v>>5)&63)*255/63;b=(v&31)*255/31;}else{r=((v>>10)&31)*255/31;g=((v>>5)&31)*255/31;b=(v&31)*255/31;}color=(r<<16)|(g<<8)|b;}pixels[y*w+x]=0xff000000|color;
 }if(testing&&!netExecuting&&!booting&&frames%120==0)render();
}
static size_t batch(const int16_t* data,size_t n){audioFrames+=n;for(size_t i=0;i<n*2;++i)if(data[i])++nonzeroAudio;
 if(audioStream&&!testing&&!booting&&!replaying){if(SDL_GetAudioStreamQueued(audioStream)>44100)SDL_ClearAudioStream(audioStream);adjustedAudio.resize(n*2);float volume=rffs::menu::settings.volume;for(size_t i=0;i<n*2;++i)adjustedAudio[i]=int16_t(data[i]*volume);SDL_PutAudioStreamData(audioStream,adjustedAudio.data(),int(n*4));}return n;}
static void audio(int16_t l,int16_t r){int16_t d[]={l,r};batch(d,1);}
static void poll(){if(netExecuting)return;rffs::inputbridge::Update();SDL_Event event;while(SDL_PollEvent(&event)){if(event.type==SDL_EVENT_QUIT)quit=true;if(!rffs::steam::OverlayVisible()&&!rffs::steam::OverlayPending()){ImGui_ImplSDL3_ProcessEvent(&event);rffs::menu::Process(event);}}rffs::menu::Update();}
static int16_t input(unsigned port,unsigned device,unsigned,unsigned id){if(port>=2||(device&RETRO_DEVICE_MASK)!=RETRO_DEVICE_JOYPAD||id>15)return 0;
 if(netExecuting)return (netInputs[port]>>id)&1;
 if(booting)return port==0&&id==RETRO_DEVICE_ID_JOYPAD_START&&!gameRunning()&&frames-bootBegin>40&&(frames-bootBegin)%30<3;
 if(testing&&port==0){if(id==RETRO_DEVICE_ID_JOYPAD_START)return (frames>=900&&frames<905)||(frames>=1900&&frames<1905);if(id==RETRO_DEVICE_ID_JOYPAD_B)return (frames>=2100&&frames<2105)||(frames>=2300&&frames<2305)||(frames>=2500&&frames<2505);}
 return rffs::menu::Input(port,id);
}
static uint64_t hashPixels(){uint64_t h=14695981039346656037ull;for(auto value:pixels){h^=value;h*=1099511628211ull;}return h;}
static void fallbackImage(){auto& cfg=rffs::menu::settings;const uint32_t* data=pixels.data();unsigned w=frameW,h=frameH;
 if(cfg.filter==2||cfg.filter==3){processed.resize(w*h*4);auto at=[&](int x,int y){return pixels[std::clamp(y,0,int(h)-1)*w+std::clamp(x,0,int(w)-1)];};for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){auto E=at(x,y),B=at(x,y-1),D=at(x-1,y),F=at(x+1,y),H=at(x,y+1);uint32_t q[4]={E,E,E,E};if(B!=H&&D!=F){q[0]=D==B?D:E;q[1]=B==F?F:E;q[2]=D==H?D:E;q[3]=H==F?F:E;}for(int j=0;j<4;++j)processed[(y*2+j/2)*(w*2)+x*2+j%2]=q[j];}w*=2;h*=2;data=processed.data();}
 if(w!=textureW||h!=textureH||!texture){if(texture)SDL_DestroyTexture(texture);texture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,w,h);check(texture!=nullptr,"Create texture");textureW=w;textureH=h;}
 SDL_SetTextureScaleMode(texture,cfg.filter==0?SDL_SCALEMODE_NEAREST:SDL_SCALEMODE_LINEAR);SDL_SetTextureColorModFloat(texture,cfg.brightness,cfg.brightness,cfg.brightness);check(SDL_UpdateTexture(texture,nullptr,data,w*4),"Upload video");int width,height;SDL_GetCurrentRenderOutputSize(renderer,&width,&height);float aspect=frameW>320?16.f/9.f:4.f/3.f;SDL_FRect dst{0,0,float(width),width/aspect};if(dst.h>height){dst.h=float(height);dst.w=height*aspect;}dst.x=(width-dst.w)/2;dst.y=(height-dst.h)/2;SDL_SetRenderDrawColor(renderer,4,5,8,255);SDL_RenderClear(renderer);SDL_RenderTexture(renderer,texture,nullptr,&dst);
 if(cfg.filter==3){SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(renderer,0,0,0,60);for(unsigned y=0;y<frameH;++y){SDL_FRect line{dst.x,dst.y+(y+.5f)*dst.h/frameH,dst.w,std::max(1.f,dst.h/frameH*.25f)};SDL_RenderFillRect(renderer,&line);}SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);}
}
static std::string screenshotRequest;
static void render(){if(pixels.empty())return;
 const bool overlay=rffs::steam::OverlayVisible()||rffs::steam::OverlayPending();static bool previousOverlay=false;
 if(overlay!=previousOverlay){SDL_SetWindowRelativeMouseMode(window,false);SDL_SetWindowMouseGrab(window,false);SDL_CaptureMouse(false);SDL_ShowCursor();auto& io=ImGui::GetIO();for(int i=0;i<5;++i)io.AddMouseButtonEvent(i,false);io.MouseDrawCursor=false;previousOverlay=overlay;}
 if(!overlay&&rffs::menu::visible){SDL_SetWindowRelativeMouseMode(window,false);SDL_SetWindowMouseGrab(window,false);}
 SDL_ShowCursor();
 auto& cfg=rffs::menu::settings;if(gpu)rffs::gpu::Draw(pixels.data(),frameW,frameH,cfg.filter,cfg.smoothing,cfg.brightness);else fallbackImage();
 // While Steam owns the mouse, keep presenting frames without running ImGui's
 // SDL mouse/cursor backend, which otherwise changes capture and hides its pointer.
 if(!overlay){if(gpu)ImGui_ImplOpenGL3_NewFrame();else ImGui_ImplSDLRenderer3_NewFrame();ImGui_ImplSDL3_NewFrame();ImGui::GetIO().MouseDrawCursor=rffs::menu::visible;ImGui::NewFrame();rffs::menu::Draw(gpu);rffs::net::HUD();rffs::menu::ShortcutHUD();ImGui::Render();if(gpu)ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());else ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),renderer);}
 if(!screenshotRequest.empty()){if(gpu)rffs::gpu::Capture(screenshotRequest.c_str());else{auto* image=SDL_RenderReadPixels(renderer,nullptr);if(image){SDL_SaveBMP(image,screenshotRequest.c_str());SDL_DestroySurface(image);}}screenshotRequest.clear();}
 if(gpu)rffs::gpu::Swap();else SDL_RenderPresent(renderer);

}

static void style(){ImGui::StyleColorsDark();auto& s=ImGui::GetStyle();s.WindowRounding=14;s.ChildRounding=10;s.FrameRounding=7;s.PopupRounding=9;s.GrabRounding=7;s.WindowPadding=ImVec2(18,16);s.FramePadding=ImVec2(12,8);s.ItemSpacing=ImVec2(12,11);s.ScrollbarSize=12;auto* c=s.Colors;c[ImGuiCol_WindowBg]=ImVec4(.035f,.05f,.08f,.98f);c[ImGuiCol_ChildBg]=ImVec4(.055f,.075f,.11f,1);c[ImGuiCol_FrameBg]=ImVec4(.10f,.14f,.2f,1);c[ImGuiCol_Button]=ImVec4(.1f,.2f,.29f,1);c[ImGuiCol_ButtonHovered]=ImVec4(.12f,.35f,.46f,1);c[ImGuiCol_ButtonActive]=ImVec4(.16f,.48f,.6f,1);c[ImGuiCol_CheckMark]=ImVec4(.25f,.85f,1,1);c[ImGuiCol_SliderGrab]=c[ImGuiCol_CheckMark];c[ImGuiCol_Header]=ImVec4(.11f,.3f,.4f,1);c[ImGuiCol_TitleBgActive]=ImVec4(.05f,.11f,.17f,1);c[ImGuiCol_Border]=ImVec4(.16f,.22f,.3f,1);}
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int){
 SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
 wchar_t exe[32768];GetModuleFileNameW(nullptr,exe,32768);fs::path root=fs::path(exe).parent_path();int argc=0;auto argv=CommandLineToArgvW(GetCommandLineW(),&argc);for(int i=1;i<argc;++i){if(!wcscmp(argv[i],L"--input-service")&&i+1<argc){auto parent=wcstoul(argv[i+1],nullptr,10);LocalFree(argv);return rffs::inputbridge::Service(parent);}if(!wcscmp(argv[i],L"--hotplug-test")){hotplugTest=true;testing=true;}if(!wcscmp(argv[i],L"--widescreen-test")){widescreenTest=true;testing=true;}if(!wcscmp(argv[i],L"--prepare-test")){preparationTest=true;testing=true;}if(!wcscmp(argv[i],L"--start-input-test")){startInputTest=true;testing=true;}if(!wcscmp(argv[i],L"--d3d11-test")){direct3DTest=true;overlayTest=true;menuTest=true;testing=true;gpuTest=true;}if(!wcscmp(argv[i],L"--overlay-test")){overlayTest=true;menuTest=true;testing=true;gpuTest=true;}if(!wcscmp(argv[i],L"--menu-test")){menuTest=true;testing=true;gpuTest=true;}if(!wcscmp(argv[i],L"--steam-check")){steamCheck=true;testing=true;}if(!wcscmp(argv[i],L"--rollback-test")){testing=true;rollbackTest=true;}if(!wcscmp(argv[i],L"--self-test"))testing=true;if(!wcscmp(argv[i],L"--self-test-gpu")){testing=true;gpuTest=true;}}LocalFree(argv);
 // Read-only packaging check. Does not initialize SDL/Steam or load game, BIOS, CHD or save state.
 if (wcsstr(GetCommandLineW(), L"--check-package") != nullptr) {
  const auto corePath = root / L"rffs-runtime/rffs_wide_v5.dll";
  const auto sdlPath = root / L"SDL3.dll";
  return (fs::is_regular_file(corePath) && fs::file_size(corePath) > 0 &&
          fs::is_regular_file(sdlPath) && fs::file_size(sdlPath) > 0) ? 0 : 2;
 }
 runtime=root/(testing?L"rffs-selftest":L"rffs-runtime");fs::create_directories(runtime/L"system");fs::current_path(root);FILE* output=nullptr;_wfreopen_s(&output,(runtime/(gpuTest?L"modern-gpu-test.log":L"modern.log")).c_str(),L"w",stdout);setvbuf(stdout,nullptr,_IONBF,0);
 try{
 if(steamCheck){check(rffs::steam::Init(),"Steam init");rffs::steam::Create();for(int i=0;i<1000&&!rffs::steam::RoomID();++i){rffs::steam::Tick();Sleep(10);}bool created=rffs::steam::RoomID()!=0;printf("STEAM_CHECK account=%s room=%llu created=%d\n",rffs::steam::Name().c_str(),static_cast<unsigned long long>(rffs::steam::RoomID()),created);rffs::steam::Shutdown();return created?0:1;}
 fs::path disc;for(const auto& e:fs::directory_iterator(root))if(e.path().extension()==L".chd"){if(!disc.empty())throw std::runtime_error("Mais de um CHD na pasta.");disc=e.path();}if(disc.empty())throw std::runtime_error("Coloque o aplicativo na mesma pasta do CHD do jogo.");auto utf8=(runtime/L"system").u8string();strcpy_s(directory,reinterpret_cast<const char*>(utf8.c_str()));
 SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI,"1");SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4,"1");SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5,"1");SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3,"1");SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS3_SIXAXIS_DRIVER,"1");SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");SDL_SetHint(SDL_HINT_XINPUT_ENABLED,"1");
 bool bridgeReady=false;if(!testing||hotplugTest){bridgeReady=rffs::inputbridge::Start(exe);printf("INPUT bridge ready=%d\n",bridgeReady);if(bridgeReady){SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI,"0");SDL_SetHint(SDL_HINT_JOYSTICK_DIRECTINPUT,"0");SDL_SetHint(SDL_HINT_JOYSTICK_RAWINPUT,"0");SDL_SetHint(SDL_HINT_JOYSTICK_WGI,"0");SDL_SetHint(SDL_HINT_XINPUT_ENABLED,"0");}}SDL_SetHint(SDL_HINT_AUTO_UPDATE_JOYSTICKS,"1");
 if(testing&&!gpuTest){SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"dummy");SDL_SetHint(SDL_HINT_AUDIO_DRIVER,"dummy");}SDL_SetMainReady();check(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMEPAD|SDL_INIT_JOYSTICK),"Initialize SDL");
 auto mappings=(root/L"rffs-runtime/gamecontrollerdb.txt").u8string();SDL_AddGamepadMappingsFromFile(reinterpret_cast<const char*>(mappings.c_str()));
 if(!testing||overlayTest||hotplugTest){rffs::steam::Init();if(rffs::steam::Active()){for(int i=0;i<200&&!GetModuleHandleW(L"GameOverlayRenderer64.dll");++i)Sleep(10);printf("[Steam] Overlay module before SDL: %d\n",GetModuleHandleW(L"GameOverlayRenderer64.dll")!=nullptr);}}
 bool preferD3D=direct3DTest||(!testing&&GetPrivateProfileIntW(L"RealBout",L"Direct3D11",1,(runtime/L"settings.ini").c_str())!=0);
 if((!testing||gpuTest)&&!preferD3D){SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);window=SDL_CreateWindow("Real Bout Special • Online v5 | F1: menu",1280,720,SDL_WINDOW_HIDDEN|SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);if(window){try{gpu=rffs::gpu::Init(window);}catch(const std::exception& e){printf("GPU fallback: %s\n",e.what());rffs::gpu::Shutdown();}if(!gpu){SDL_DestroyWindow(window);window=nullptr;}}}
 if(!window){window=SDL_CreateWindow("Real Bout Special • Online v5 | F1: menu",1280,720,SDL_WINDOW_HIDDEN|SDL_WINDOW_RESIZABLE);check(window!=nullptr,"Create window");renderer=SDL_CreateRenderer(window,preferD3D?"direct3d11":(testing?"software":nullptr));check(renderer!=nullptr,"Create renderer");SDL_SetRenderVSync(renderer,0);}if(gpuTest&&!gpu&&!preferD3D)throw std::runtime_error("Teste GPU requer OpenGL 3.3.");
 if(auto hwnd=static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(window),SDL_PROP_WINDOW_WIN32_HWND_POINTER,nullptr))){auto instance=GetModuleHandleW(nullptr);SendMessageW(hwnd,WM_SETICON,ICON_BIG,reinterpret_cast<LPARAM>(LoadImageW(instance,MAKEINTRESOURCEW(1),IMAGE_ICON,256,256,LR_SHARED)));SendMessageW(hwnd,WM_SETICON,ICON_SMALL,reinterpret_cast<LPARAM>(LoadImageW(instance,MAKEINTRESOURCEW(1),IMAGE_ICON,32,32,LR_SHARED)));}
 printf("VIDEO_BACKEND=%s\n",gpu?"OpenGL":SDL_GetRendererName(renderer));
 IMGUI_CHECKVERSION();ImGui::CreateContext();ImGui::GetIO().IniFilename=nullptr;ImGui::GetIO().ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard|ImGuiConfigFlags_NoMouseCursorChange;wchar_t windowsPath[MAX_PATH];GetWindowsDirectoryW(windowsPath,MAX_PATH);auto fontPath=(fs::path(windowsPath)/L"Fonts/segoeui.ttf").string();static const ImWchar ranges[]={0x20,0xff,0x2022,0x2022,0};ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath.c_str(),19,nullptr,ranges);style();
 if(gpu){ImGui_ImplSDL3_InitForOpenGL(window,SDL_GL_GetCurrentContext());ImGui_ImplOpenGL3_Init("#version 330 core");}else{ImGui_ImplSDL3_InitForSDLRenderer(window,renderer);ImGui_ImplSDLRenderer3_Init(renderer);}rffs::inputbridge::Update();rffs::menu::Init(window,(runtime/L"settings.ini").wstring(),testing);check(SDL_SetCursor(SDL_GetDefaultCursor()),"Default system cursor");check(SDL_ShowCursor(),"Show system cursor");if(rffs::menu::settings.fullscreen&&!testing)rffs::menu::ApplyWindowMode(true);
 auto module=LoadLibraryExW((runtime/L"rffs_wide_v5.dll").c_str(),nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);if(!module)throw std::runtime_error("Não foi possível carregar rffs-runtime/rffs_wide_v5.dll.");
#define API(n) auto n=reinterpret_cast<decltype(&retro_##n)>(GetProcAddress(module,"retro_" #n));if(!n)throw std::runtime_error("Core sem API " #n)
 API(set_environment);API(set_video_refresh);API(set_audio_sample);API(set_audio_sample_batch);API(set_input_poll);API(set_input_state);API(set_controller_port_device);API(init);API(load_game_special);API(get_system_av_info);API(run);API(reset);API(serialize_size);API(serialize);API(unserialize);API(unload_game);API(deinit);
 guestPC=reinterpret_cast<decltype(guestPC)>(GetProcAddress(module,"?SekGetPC@@YAIH@Z"));set_environment(env);set_video_refresh(video);set_audio_sample(audio);set_audio_sample_batch(batch);set_input_poll([]{});set_input_state(input);init();auto discUtf8=disc.u8string();retro_game_info game{reinterpret_cast<const char*>(discUtf8.c_str()),nullptr,0,nullptr};if(!load_game_special(13,&game,1)||errors)throw std::runtime_error("Falha ao iniciar. Veja rffs-runtime/modern.log.");for(unsigned p=0;p<2;++p)set_controller_port_device(p,RETRO_DEVICE_JOYPAD);retro_system_av_info av{};get_system_av_info(&av);SDL_AudioSpec spec{SDL_AUDIO_S16,2,int(av.timing.sample_rate)};audioStream=SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&spec,nullptr,nullptr);check(audioStream!=nullptr,"Open audio");check(SDL_ResumeAudioStreamDevice(audioStream),"Start audio");
 nativeScreenWidth=reinterpret_cast<int*>(GetProcAddress(module,"?nNeoScreenWidth@@3HA"));auto nativePitch=reinterpret_cast<int*>(GetProcAddress(module,"nBurnPitch"));auto nativeBpp=reinterpret_cast<int*>(GetProcAddress(module,"nBurnBpp"));check(nativeScreenWidth&&nativePitch&&nativeBpp,"Native video dimensions");
 auto runFrame=[&]{*nativeScreenWidth=rffs::menu::settings.widescreen?426:320;*nativePitch=*nativeScreenWidth**nativeBpp;run();};
 auto startDirect=[&]{if(!rffs::menu::settings.directBoot)return;booting=true;bootBegin=frames;int ready=0;for(int i=0;i<2400&&!quit;++i){runFrame();ready=gameRunning()?ready+1:0;if(ready>=100)break;}booting=false;printf("DIRECT_BOOT frames=%d game=%d pc=%06x\n",frames-int(bootBegin),gameRunning(),guestPC?guestPC(0):0);SDL_ClearAudioStream(audioStream);};startDirect();
 auto saveCore=[&]{std::vector<uint8_t> state(serialize_size()+sizeof(frames));memcpy(state.data(),&frames,sizeof(frames));check(serialize(state.data()+sizeof(frames),state.size()-sizeof(frames)),"Netplay save");return state;};
 auto loadCore=[&](const std::vector<uint8_t>& state){if(state.size()!=serialize_size()+sizeof(frames))return false;memcpy(&frames,state.data(),sizeof(frames));SDL_ClearAudioStream(audioStream);return unserialize(state.data()+sizeof(frames),state.size()-sizeof(frames));};
 auto gameHash=[&]{uint64_t h=14695981039346656037ull;for(size_t i=0;i<programRamSize;++i){h^=programRam[i];h*=1099511628211ull;}return h;};
 auto netFrame=[&](const uint16_t* buttons,bool replay){netInputs[0]=buttons[0];netInputs[1]=buttons[1];netExecuting=true;replaying=replay;runFrame();netExecuting=false;replaying=false;};
 uint64_t compatibility=14695981039346656037ull;auto hashFile=[&](const fs::path& file,size_t limit){std::ifstream f(file,std::ios::binary);if(!f)throw std::runtime_error("Arquivo de compatibilidade ausente");char block[65536];while(f&&limit){f.read(block,std::streamsize(std::min(limit,sizeof(block))));auto count=f.gcount();for(std::streamsize i=0;i<count;++i){compatibility^=uint8_t(block[i]);compatibility*=1099511628211ull;}limit-=size_t(count);}};
 hashFile(disc,124);hashFile(runtime/L"rffs_wide_v5.dll",SIZE_MAX);hashFile(runtime/L"system/fbneo/neocdz.zip",SIZE_MAX);
 auto initialStateFile=[&]{return fs::exists(runtime/L"estado-manual.rffsstate")?L"estado-manual.rffsstate":L"estado-inicial-protegido.rffsstate";};
 std::vector<uint8_t> launchState;
 if(!testing||preparationTest||widescreenTest){auto fallback=saveCore();try{auto saved=readManualState(runtime,compatibility,fallback.size(),initialStateFile());if(!saved.empty()){if(!loadCore(saved)){check(loadCore(fallback),"Restore normal startup");throw std::runtime_error("Não foi possível carregar o estado; início normal preservado.");}launchState=saved;rffs::menu::Notify(fs::exists(runtime/L"estado-manual.rffsstate")?"Estado inicial carregado":"Backup do estado inicial carregado");printf("MANUAL_STATE_AUTOLOAD_PASS\n");}}catch(const std::exception& e){rffs::menu::Notify(e.what());printf("MANUAL_STATE_AUTOLOAD: %s\n",e.what());}}
 rffs::net::Init({saveCore,loadCore,netFrame,gameHash},launchState.empty()?saveCore():launchState,compatibility);
 if(!testing)SDL_ShowWindow(window);
 if(menuTest){
 rffs::menu::ApplyWindowMode(true);check((SDL_GetWindowFlags(window)&SDL_WINDOW_BORDERLESS)!=0&&(SDL_GetWindowFlags(window)&SDL_WINDOW_FULLSCREEN)==0,"Borderless regular window");rffs::menu::ApplyWindowMode(false);printf("BORDERLESS_PASS regular window, no SDL fullscreen\n");
 if(overlayTest){for(int i=0;i<120;++i){rffs::steam::Tick();render();Sleep(16);}printf("OVERLAY_TEST ready=%d\n",rffs::steam::OverlayReady());}
 rffs::menu::visible=false;SDL_Event e{};e.type=SDL_EVENT_KEY_DOWN;e.key.windowID=SDL_GetWindowID(window);e.key.key=SDLK_F1;e.key.scancode=SDL_SCANCODE_F1;SDL_PushEvent(&e);poll();check(rffs::menu::visible,"F1 opens menu");int frozen=frames;
 for(int i=0;i<8;++i){poll();ImGui::GetIO().AddMousePosEvent(640+i*10.f,360);render();}
 check(frames==frozen,"Menu pauses simulation");check(ImGui::GetIO().MouseDrawCursor,"F1 software pointer visible");check((ImGui::GetIO().ConfigFlags&ImGuiConfigFlags_NoMouseCursorChange)!=0&&SDL_CursorVisible(),"Native cursor preserved alongside F1");check(!SDL_GetWindowRelativeMouseMode(window)&&!SDL_GetWindowMouseGrab(window),"Mouse released");screenshotRequest="rffs-selftest/menu-f1-fix.bmp";render();
 e.type=SDL_EVENT_KEY_UP;SDL_PushEvent(&e);e.type=SDL_EVENT_KEY_DOWN;SDL_PushEvent(&e);poll();check(!rffs::menu::visible,"F1 closes menu");render();printf("MENU_F1_PASS open, render loop, pointer, mouse release, close\n");
 }else if(hotplugTest){check(rffs::inputbridge::SelfTest(),"Hotplug under Steam");
 }else if(widescreenTest){
 auto before=saveCore();netExecuting=true;netInputs[0]=netInputs[1]=0;
 rffs::menu::settings.widescreen=false;for(int i=0;i<120;++i)runFrame();check(frameW==320&&*nativeScreenWidth==320&&*nativePitch==320**nativeBpp,"Original 4:3 dimensions");auto normalHash=gameHash();
 check(loadCore(before),"Restore before wide comparison");rffs::menu::settings.widescreen=true;for(int i=0;i<120;++i)runFrame();check(frameW==426&&*nativeScreenWidth==426&&*nativePitch==426**nativeBpp,"Wide 16:9 dimensions");check(gameHash()==normalHash,"Video mode preserves game RAM and timing");
 check(loadCore(before),"Restore before second normal mode");rffs::menu::settings.widescreen=false;for(int i=0;i<120;++i)runFrame();check(gameHash()==normalHash&&frameW==320,"Wide to normal repeat");netExecuting=false;check(loadCore(before),"Restore after wide test");printf("WIDESCREEN_TOGGLE_PASS 320 -> 426 -> 320, game RAM identical\n");
 }else if(preparationTest){check(rffs::net::PreparationSelfTest(),"Preparation test");
 }else if(startInputTest){
 auto diag=reinterpret_cast<uint8_t**>(GetProcAddress(module,"?diag_input@@3PEAEEA"));
 auto joy=reinterpret_cast<uint8_t*>(GetProcAddress(module,"?NeoJoy1@@3PAEA"));
 auto buttons=reinterpret_cast<uint8_t*>(GetProcAddress(module,"?NeoButton1@@3PAEA"));
 check(diag&&joy&&buttons,"Core input diagnostics exports");check(!*diag,"Hold Start diagnostic shortcut disabled");
 auto before=saveCore();netExecuting=true;netInputs[0]=(1u<<3)|(1u<<8)|(1u<<1);netInputs[1]=0;
 for(int i=0;i<150;++i){runFrame();check(buttons[0]&&joy[5]&&joy[6],"Start+B+C remains active past diagnostic timeout");}
 netInputs[0]=0;runFrame();check(!buttons[0]&&!joy[5]&&!joy[6],"Start+B+C released");netExecuting=false;check(loadCore(before),"Restore after Start test");
 auto protectedBefore=readManualState(runtime,compatibility,saveCore().size(),L"estado-inicial-protegido.rffsstate");auto state=saveCore();writeManualState(runtime,compatibility,state);check(readManualState(runtime,compatibility,state.size(),L"estado-inicial-protegido.rffsstate")==protectedBefore,"F5 preserves protected initial state");auto disk=readManualState(runtime,compatibility,state.size());check(disk==state,"Manual state disk roundtrip");auto expectedHash=gameHash();netExecuting=true;runFrame();netExecuting=false;check(loadCore(disk)&&gameHash()==expectedHash,"Manual state restore game RAM");bool rejected=false;try{readManualState(runtime,compatibility^1,state.size());}catch(...){rejected=true;}check(rejected,"Reject incompatible state");printf("MANUAL_STATE_PASS atomic save and byte-identical readback\n");
 printf("START_INPUT_PASS 150 frames Start+B+C, release, state restored\n");
 }else if(rollbackTest){check(rffs::net::SelfTest(1200),"GGPO rollback test");}
 else if(testing){check(rffs::menu::InputSelfTest(),"Input self-test");for(int i=frames;i<4000;++i){runFrame();}printf("BIOS_REGION=%u (Japan=0)\n",programRam?programRam[0x10fd83^1]:255);check(programRam&&programRam[0x10fd83^1]==0,"Japanese BIOS region");
  for(int filter=0;filter<4;++filter){rffs::menu::settings.filter=filter;screenshotRequest="rffs-selftest/filter-"+std::to_string(filter)+(gpu?"-gpu.bmp":"-compatible.bmp");render();}
  rffs::menu::visible=true;for(int t=0;t<4;++t){rffs::menu::SetTab(t);render();screenshotRequest="rffs-selftest/menu-"+std::to_string(t)+(gpu?"-gpu.bmp":"-compatible.bmp");render();}rffs::menu::visible=false;
  std::vector<uint8_t> state(serialize_size());check(serialize(state.data(),state.size()),"Save state");int baseline=frames;std::vector<uint64_t> reference;for(int i=0;i<60;++i){runFrame();reference.push_back(hashPixels());}check(unserialize(state.data(),state.size()),"Restore state");frames=baseline;for(int i=0;i<60;++i){runFrame();if(hashPixels()!=reference[i])throw std::runtime_error("State replay diverged");}reset();frames=0;startDirect();check(gameRunning(),"Direct boot after reset");if(errors||!audioFrames||!nonzeroAudio)throw std::runtime_error("Audio/video validation failed");printf("PASS: direct boot, keyboard/input isolation, 4 filters, 3 menu tabs, audio, state replay; GPU=%d\n",gpu);
 }else{auto next=std::chrono::steady_clock::now();auto period=std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(1./av.timing.fps));bool paused=false;while(!quit&&!rffs::menu::quitRequested){poll();rffs::net::Tick();static uint64_t seenRoom=0;auto room=rffs::steam::RoomID();if(room&&room!=seenRoom){rffs::menu::visible=true;rffs::menu::SetTab(3);}seenRoom=room;bool online=rffs::net::Locked();bool menu=rffs::menu::visible;if(rffs::menu::loadStateRequested){rffs::menu::loadStateRequested=false;if(online)rffs::menu::Notify("Carregue o estado fora da partida online.");else{try{auto backup=saveCore();auto saved=readManualState(runtime,compatibility,backup.size(),initialStateFile());if(saved.empty())rffs::menu::Notify("Estado não encontrado. Extraia a atualização.");else{if(!loadCore(saved)){check(loadCore(backup),"Restore after failed state load");throw std::runtime_error("Falha ao carregar estado; partida anterior restaurada.");}rffs::net::SetInitial(saved);rffs::menu::Notify("Estado inicial carregado (F6)");printf("MANUAL_STATE_F6_LOADED\n");next=std::chrono::steady_clock::now();}}catch(const std::exception& e){rffs::menu::Notify(e.what());}}}if(rffs::menu::saveStateRequested){rffs::menu::saveStateRequested=false;if(online)rffs::menu::Notify("Salve o estado fora da partida online.");else{try{auto snapshot=saveCore();writeManualState(runtime,compatibility,snapshot);rffs::net::SetInitial(snapshot);rffs::menu::Notify("Estado permanente salvo (F5)");printf("MANUAL_STATE_SAVED\n");}catch(const std::exception& e){rffs::menu::Notify(e.what());}}}if(menu&&!paused)SDL_ClearAudioStream(audioStream);paused=menu;if(rffs::menu::restartRequested&&!online){rffs::menu::restartRequested=false;reset();frames=0;startDirect();next=std::chrono::steady_clock::now();}if(online){uint16_t buttons=0;for(unsigned id=0;id<16;++id)if(rffs::menu::Input(0,id))buttons|=uint16_t(1u<<id);if(rffs::net::Step(buttons))rffs::menu::AdvanceInput();}else if(!menu){runFrame();rffs::menu::AdvanceInput();}render();next+=period;auto now=std::chrono::steady_clock::now();if(now-next>std::chrono::milliseconds(100))next=now;std::this_thread::sleep_until(next);}}
 rffs::net::Shutdown();
 unload_game();deinit();rffs::menu::Shutdown();if(gpu)ImGui_ImplOpenGL3_Shutdown();else ImGui_ImplSDLRenderer3_Shutdown();ImGui_ImplSDL3_Shutdown();ImGui::DestroyContext();SDL_DestroyAudioStream(audioStream);if(texture)SDL_DestroyTexture(texture);if(renderer)SDL_DestroyRenderer(renderer);rffs::gpu::Shutdown();SDL_DestroyWindow(window);rffs::inputbridge::Stop();SDL_Quit();FreeLibrary(module);return 0;
 }catch(const std::exception& e){printf("ERROR: %s\n",e.what());if(!testing)MessageBoxA(nullptr,e.what(),"Real Bout Special",MB_OK|MB_ICONERROR);rffs::inputbridge::Stop();SDL_Quit();return 1;}
}