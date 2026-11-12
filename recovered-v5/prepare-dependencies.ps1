# Instala as dependências de código aberto em pastas locais da V5.
# Não modifica os binários atuais da Release nem instala ROM/BIOS.
param([switch]$Build)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
function RunChecked([scriptblock]$cmd) {
  & $cmd
  if ($LASTEXITCODE -ne 0) { throw "Comando retornou erro: $LASTEXITCODE" }
}
function Checkout($name, $url, $commit) {
  $dir = Join-Path $root $name
  if (-not (Test-Path $dir)) {
    git clone --filter=blob:none $url $dir
    if ($LASTEXITCODE -ne 0) { throw "git clone falhou: $name" }
  }
  git -C $dir fetch origin $commit
  if ($LASTEXITCODE -ne 0) { throw "git fetch falhou: $name" }
  git -C $dir checkout --detach $commit
  if ($LASTEXITCODE -ne 0) { throw "git checkout falhou: $name" }
}
Checkout 'FBNeo' 'https://github.com/libretro/FBNeo.git' '9d82650bb9591966987a4c36ae8a1409ce0d5d68'
Checkout 'ggpo' 'https://github.com/pond3r/ggpo.git' '7ddadef8546a7d99ff0b3530c6056bc8ee4b9c0a'
Checkout 'imgui' 'https://github.com/ocornut/imgui.git' '6d910d5487d11ca567b61c7824b0c78c569d62f0'
$patches = @(
  @{ folder='FBNeo'; file='FBNeo-widescreen.patch' },
  @{ folder='ggpo'; file='GGPO-Steam.patch' }
)
foreach($p in $patches) {
  $dir=Join-Path $root $p.folder
  $patch=Join-Path $root $p.file
  git -C $dir apply --check $patch
  if($LASTEXITCODE -eq 0) {
    git -C $dir apply $patch
    if($LASTEXITCODE -ne 0) { throw "Erro aplicando patch $patch" }
  } else {
    git -C $dir apply --reverse --check $patch
    if($LASTEXITCODE -ne 0) { throw "Patch incompatível: $patch" }
    Write-Host "Patch já aplicado: $patch"
  }
}
$sdlDir=Join-Path $root 'SDL3'
$sdlZip=Join-Path $root 'SDL3-3.4.18.zip'
$sdlTarget=Join-Path $sdlDir 'SDL3-3.4.18'
if(-not (Test-Path $sdlTarget)) {
  New-Item -ItemType Directory -Force -Path $sdlDir | Out-Null
  Invoke-WebRequest 'https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-devel-3.4.18-VC.zip' -OutFile $sdlZip
  Expand-Archive -LiteralPath $sdlZip -DestinationPath $sdlDir -Force
}
Write-Host 'Fontes e bibliotecas preparadas.' -ForegroundColor Green
if($Build) {
  & python (Join-Path $root 'configure_core.py')
  if($LASTEXITCODE -ne 0) {throw 'Falha em configure_core.py'}
  cmake -S $root -B (Join-Path $root 'core-build') -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_BUILD_TYPE=Release
  if($LASTEXITCODE -ne 0){throw 'Falha ao configurar o núcleo'}
  cmake --build (Join-Path $root 'core-build') --parallel 4
  if($LASTEXITCODE -ne 0){throw 'Falha ao compilar o núcleo'}
  & (Join-Path $root 'build_player.ps1') -OutputDirectory (Join-Path $root 'runtime')
  if($LASTEXITCODE -ne 0){throw 'Falha ao compilar o player'}
}
