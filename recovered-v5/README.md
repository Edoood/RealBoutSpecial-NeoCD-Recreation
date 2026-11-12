# Fontes recuperadas da versão V5

Estes arquivos foram recuperados do arquivo de desenvolvimento `teste.7z` fornecido pelo mantenedor. **Não são um projeto CMake autocontido nem está comprovado que geram binários idênticos aos distribuídos na versão V6.**

## Arquivos relevantes

- `FBNeo-widescreen.patch`: modificações no núcleo FinalBurn Neo para a viewport widescreen
- `GGPO-Steam.patch`: alterações na integração GGPO
- `configure_core.py`: gera o `CMakeLists.txt` do núcleo a partir das fontes do FBNeo
- `rffs_player.cpp`, `rffs_net.cpp`, `rffs_steam.cpp`, `rffs_menu.cpp`, `rffs_gl.cpp`, `rffs_input_bridge.cpp`: código-fonte recuperado do player
- `rffs_*.h`: cabeçalhos do player
- `build_player.ps1`: script histórico; exige uma subpasta `player/` com `CMakeLists.txt`, ainda não recuperada
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

**Etapa 5 ainda não está reproduzível a partir deste repositório:** o script `build_player.ps1` aponta para `player/CMakeLists.txt`, que não foi localizado. O projeto original também referenciava recursos auxiliares como `rffs.ico`.

## Integridade e compatibilidade

Os binários do ZIP funcional **não devem ser substituídos** por novos executáveis até que exista uma compilação testada. Mesmo compilando as fontes V5, compatibilidade com o save state e com o protocolo do Netplay da V6 precisa ser verificada.

A publicação das fontes e patches ajuda a cumprir exigências de transparência, mas por si só não atesta conformidade com todas as licenças nem resolve condições de redistribuição do Steamworks ou possíveis dados protegidos no save state.
