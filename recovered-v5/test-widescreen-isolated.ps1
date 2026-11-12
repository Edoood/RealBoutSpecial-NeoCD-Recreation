# Functional validation in a NEW folder, never modifying the working V6 game.
# Usage example:
# .\test-widescreen-isolated.ps1 -ExperimentalPackageFolder 'D:\testes\novo\test-package' -ChdFile 'D:\jogo\disc.chd' -BiosFile 'D:\jogo\Support\rffs-runtime\system\fbneo\neocdz.zip' -OutputFolder 'D:\rffs-test-isolado'
[CmdletBinding()]
param(
  [Parameter(Mandatory=$true)][string]$ExperimentalPackageFolder,
  [Parameter(Mandatory=$true)][string]$ChdFile,
  [Parameter(Mandatory=$true)][string]$BiosFile,
  [Parameter(Mandatory=$true)][string]$OutputFolder,
  [switch]$FullTest
)
$ErrorActionPreference='Stop'
$source=(Resolve-Path -LiteralPath $ExperimentalPackageFolder).Path
$chd=(Resolve-Path -LiteralPath $ChdFile).Path
$bios=(Resolve-Path -LiteralPath $BiosFile).Path
$dest=[IO.Path]::GetFullPath($OutputFolder)
if (Test-Path -LiteralPath $dest) { throw "Destino ja existe: $dest. Escolha uma pasta NOVA para impedir sobrescritas." }
if ([IO.Path]::GetExtension($chd).ToLowerInvariant() -ne '.chd') {throw "Arquivo CHD esperado: $chd"}
if ([IO.Path]::GetFileName($bios).ToLowerInvariant() -ne 'neocdz.zip') {throw "BIOS neocdz.zip esperada: $bios"}
$exe=Join-Path $source 'RFFS-Online-v5.exe'
$core=Join-Path $source 'rffs-runtime\rffs_wide_v5.dll'
$sdl=Join-Path $source 'SDL3.dll'
foreach($p in @($exe,$core,$sdl)){if(!(Test-Path -LiteralPath $p -PathType Leaf)){throw "Pacote experimental incompleto: $p"}}
$destRoot=[IO.Path]::GetFullPath($dest).TrimEnd('\') + '\'
foreach($p in @($source,$chd,$bios)) {
  $resolved=[IO.Path]::GetFullPath($p)
  if($resolved.StartsWith($destRoot,[StringComparison]::OrdinalIgnoreCase)) {throw "Destino nao pode conter arquivos-fonte."}
}
New-Item -ItemType Directory -Path $dest -Force | Out-Null
$testRuntime=Join-Path $dest 'rffs-selftest'
$biosFolder=Join-Path $testRuntime 'system\fbneo'
New-Item -ItemType Directory -Path $biosFolder -Force | Out-Null
# Only the reconstructed EXE/DLL and SDL3; never copy V6's EXE, DLL, protected state or config.
Copy-Item -LiteralPath $exe -Destination $dest
Copy-Item -LiteralPath $sdl -Destination $dest
Copy-Item -LiteralPath $core -Destination (Join-Path $testRuntime 'rffs_wide_v5.dll')
Copy-Item -LiteralPath $chd -Destination (Join-Path $dest ([IO.Path]::GetFileName($chd)))
Copy-Item -LiteralPath $bios -Destination (Join-Path $biosFolder 'neocdz.zip')
$player=Join-Path $dest 'RFFS-Online-v5.exe'
$tests=if($FullTest){@('--widescreen-test','--self-test')}else{@('--widescreen-test')}
$results=@()
foreach($test in $tests) {
  Write-Host "Running isolated test: $test"
  $p=Start-Process -FilePath $player -ArgumentList $test -WorkingDirectory $dest -Wait -PassThru
  $log=Join-Path $testRuntime 'modern.log'
  $text=if(Test-Path -LiteralPath $log){Get-Content -LiteralPath $log -Raw}else{"MISSING LOG"}
  $copy=Join-Path $dest ("result-"+$test.TrimStart('-')+".log")
  $text | Set-Content -LiteralPath $copy -Encoding UTF8
  $results+=("{0}: exit={1}, log={2}" -f $test,$p.ExitCode,$copy)
  if($p.ExitCode -ne 0){Write-Host ($results -join "`n");throw "Falha no teste $test. Verifique o log."}
  if($test -eq '--widescreen-test' -and $text -notmatch 'WIDESCREEN_TOGGLE_PASS') {throw "Teste terminou sem registrar WIDESCREEN_TOGGLE_PASS: $copy"}
  if($test -eq '--self-test' -and $text -notmatch 'PASS: direct boot') {throw "Self-test nao informou sucesso: $copy"}
}
$results -join "`n" | Set-Content -LiteralPath (Join-Path $dest 'test-results.txt') -Encoding UTF8
Write-Host "PASS: testes funcionais da versao experimental, isolados em $dest"
Write-Host "Nao demonstra compatibilidade da DLL reconstruida com save state/Netplay da V6."
