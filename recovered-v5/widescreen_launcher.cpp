#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <filesystem>
#include <string>
int WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR arguments,int){
 wchar_t ownPath[32768];DWORD size=GetModuleFileNameW(nullptr,ownPath,32768);
 if(!size||size>=32768)return 2;
 auto folder=std::filesystem::path(ownPath).parent_path();
 auto executable=folder/L"Real Bout Special v4.exe";
 if(!std::filesystem::is_regular_file(executable)||!std::filesystem::is_regular_file(folder/L"rffs-runtime/rffs_wide_v4.dll")){
  MessageBoxW(nullptr,L"Faltam arquivos da versao widescreen atual. Mantenha o iniciador na pasta do jogo.",L"Real Bout Special",MB_OK|MB_ICONERROR);return 2;
 }
 if(!wcscmp(arguments,L"--check"))return 0;
 std::wstring command=L"\""+executable.wstring()+L"\"";
 STARTUPINFOW info{};info.cb=sizeof(info);PROCESS_INFORMATION process{};
 if(!CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,FALSE,0,nullptr,folder.c_str(),&info,&process)){
  MessageBoxW(nullptr,L"Nao foi possivel iniciar o jogo.",L"Real Bout Special",MB_OK|MB_ICONERROR);return 1;
 }
 CloseHandle(process.hThread);CloseHandle(process.hProcess);return 0;
}