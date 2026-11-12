@echo off
setlocal EnableExtensions
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo ERROR: vswhere.exe not found
  exit /b 1
)
set "VSPATH="
for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%I"
if not defined VSPATH (
  echo ERROR: Visual Studio with MSVC tools not found
  exit /b 1
)
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
if not exist build mkdir build
cl /nologo /std:c++17 /EHsc /MD /O2 /W4 /DUNICODE /D_UNICODE src\RealBoutLauncher.cpp /Fe:build\RealBoutLauncher.exe /link /SUBSYSTEM:WINDOWS /MACHINE:X64
if errorlevel 1 exit /b 1
echo Build succeeded.