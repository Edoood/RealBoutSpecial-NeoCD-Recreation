param([string]$OutputDirectory="$PSScriptRoot/runtime", [switch]$Widescreen)
$ErrorActionPreference='Stop'
$buildDirectory=Join-Path $PSScriptRoot 'player-build'
& cmake -S (Join-Path $PSScriptRoot 'player') -B $buildDirectory -G Ninja -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release
if($LASTEXITCODE -ne 0){throw 'Falha ao preparar a compilação'}
& cmake --build $buildDirectory --parallel 4
if($LASTEXITCODE -ne 0){throw 'Falha ao compilar o aplicativo'}
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
Copy-Item -LiteralPath (Join-Path $buildDirectory 'RFFS-Online-v5.exe') -Destination $OutputDirectory
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'SDL3/SDL3-3.4.18/lib/x64/SDL3.dll') -Destination $OutputDirectory