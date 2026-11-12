# Fontes recuperadas da versão V5

Estes arquivos foram recuperados do arquivo de desenvolvimento `teste.7z` fornecido pelo mantenedor. **Não são um projeto CMake autocontido nem está comprovado que geram binários idênticos aos distribuídos na versão V6.**

## Arquivos relevantes

- `FBNeo-widescreen.patch`: modificações no núcleo FinalBurn Neo para a viewport widescreen
- `GGPO-Steam.patch`: alterações na integração GGPO
- `configure_core.py`: gera o `CMakeLists.txt` do núcleo a partir das fontes do FBNeo
- `rffs_player.cpp`, `rffs_net.cpp`, `rffs_steam.cpp`, `rffs_menu.cpp`, `rffs_gl.cpp`, `rffs_input_bridge.cpp`: código-fonte recuperado do player
- `rffs_*.h`: cabeçalhos do player
- `build_player.ps1`: script recuperado; usa `player/CMakeLists.txt` reconstruído para compilar o player experimental
- `DEPENDENCIAS.txt`: referências e revisões das dependências originais
- `ONLINE-V5.txt`: comportamento e limites dos testes documentados pela versão V5

## Obter os componentes originais

A documentação recuperada registra estes pontos de partida:

- FBNeo: https://github.com/libretro/FBNeo , commit `9d82650bb9591966987a4c36ae8a1409ce0d5d68`
- GGPO: https://github.com/pond3r/ggpo , commit `7ddadef8546a7d99ff0b3530c6056bc8ee4b9c0a`
- Dear ImGui: https://github.com/ocornut/imgui , commit `6d910d5487d11ca567b61c7824b0c78c569d62f0`
- SDL3 VC SDK: versão 3.4.18, conforme `DEPENDENCIAS.txt`

Essas são **referências históricas fornecidas nos arquivos recuperados**, não validações de compilação atuais.

## Procedimento histórico documentado

1. Fazer checkout das revisões exatas do FBNeo e GGPO.
2. Aplicar `FBNeo-widescreen.patch` e `GGPO-Steam.patch` nas respectivas árvores de código.
3. Executar `configure_core.py` para gerar o projeto do núcleo modificado.
4. Compilar o núcleo com CMake, Ninja e compilador Windows compatível.
5. Compilar o player com os arquivos `rffs_*.cpp` e as bibliotecas correspondentes.

**Atualização:** um `player/CMakeLists.txt` foi reconstruído com os arquivos de player recuperados, além de `prepare-dependencies.ps1` para baixar versões históricas de FBNeo, GGPO, ImGui e SDL3. **A reconstrução já compilou com sucesso no GitHub Actions em 12/11/2026** (execução [43140522509](https://github.com/Edoood/RealBoutSpecial-NeoCD-Recreation/actions/runs/43140522509)). Isso confirma a compilação, mas **não comprova que reproduz a V6 funcional** nem que os testes de jogo, savestate ou multiplayer passaram. O projeto original também referenciava recursos auxiliares como `rffs.ico`.

## Integridade e compatibilidade

Os binários do ZIP funcional **não devem ser substituídos** por novos executáveis até que exista uma compilação testada. Mesmo compilando as fontes V5, compatibilidade com o save state e com o protocolo do Netplay da V6 precisa ser verificada.

A publicação das fontes e patches ajuda a cumprir exigências de transparência, mas por si só não atesta conformidade com todas as licenças nem resolve condições de redistribuição do Steamworks ou possíveis dados protegidos no save state.

## Reconstrução experimental (sem substituir a Release)

Na pasta `recovered-v5` do repositório, em uma cópia de desenvolvimento, use `powershell -ExecutionPolicy Bypass -File .\prepare-dependencies.ps1 -Build`. O script obtém as revisões indicadas em `DEPENDENCIAS.txt`, aplica patches e tenta compilar o núcleo e o player. Requer Git, CMake, Ninja, clang-cl e Python, além das ferramentas de build para Windows. **Compilação automática bem-sucedida no GitHub Actions; execução do jogo e compatibilidade com a V6 ainda não verificadas.** Não substitua os arquivos da Release por um build experimental: a compatibilidade de savestate e Netplay exige testes próprios.

## Verificação técnica sem modificar o ZIP original

O player recuperado carrega `rffs-runtime/rffs_wide_v5.dll` através de `LoadLibraryExW`, enquanto o CMake reconstruído gera `rffs_core.dll`. Uma configuração experimental nova no GitHub Actions inspeciona os símbolos exportados pela DLL com `dumpbin` e grava `runtime/compatibility-report.txt` no artefato. Este relatório é **uma análise estática**, não um teste de execução.

O formato do estado salvo utiliza a assinatura `RFFSST02` e uma chave calculada sobre os primeiros 124 bytes do CHD, **todos os bytes da DLL** e a BIOS `neocdz.zip`. Por isso, uma DLL recompilada pode invalidar `estado-inicial-protegido.rffsstate`, mesmo com API compatível. Não renomeie nem substitua o DLL original dentro da V6; mantenha a versão experimental isolada e jamais modifique o ZIP da Release só com base no sucesso da compilação.
