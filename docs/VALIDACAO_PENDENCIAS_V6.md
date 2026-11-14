# Validação das pendências da V6 — 14/11/2026

| Item | Resultado verificado | Evidência que falta |
| --- | --- | --- |
| CHD e BIOS | O inventário anterior, vinculado ao SHA-256 do ZIP original, não lista CHD ou BIOS. | Para qualquer novo ZIP, repetir o inventário sobre o arquivo final. |
| Avisos de licença | O rascunho lista dois anexos. O original não traz os avisos. A auditoria anterior registrou FBNeo, GGPO, SDL3 e ImGui na cópia revisada. | Conferir os avisos adicionais de cada componente efetivamente distribuído. |
| Fonte do núcleo | O patch local recuperado e o patch no GitHub têm o mesmo identificador Git `c90934f7e6c5d14fb16dc3ec2d53e239e1131043`. O patch define e usa `RffsWideSprites[384]`. O arquivo de desenvolvimento `teste.7z` contém fontes do player e um patch widescreen. | Identificar o snapshot e os insumos que produziram a DLL e o EXE da V6. O nome exportado e a identidade do patch com a cópia do GitHub não vinculam o patch ao binário da V6. |
| Savestate | O inventário registra 8.974.473 bytes. A busca local encontrou dois SHA-256 distintos entre estados com esse tamanho. O player recuperado grava um estado serializado do núcleo, em formato próprio com checagem de compatibilidade. | Confirmar o conteúdo das regiões serializadas e confirmar a base de direitos para distribuir o conteúdo. |
| Arte e controles | O inventário registra `Support/banner.bmp` e `Support/rffs-runtime/gamecontrollerdb.txt`. O projeto upstream do banco conhecido publica licença própria. | Identificar a origem e a autorização do banner, e vincular o banco entregue a uma revisão upstream com seu aviso de licença. |
| Visual C++ | A Microsoft prevê redistribuição sujeita aos termos da licença Visual Studio e à lista REDIST da versão aplicável. | Identificar versão, origem e condições aplicáveis às três DLLs do ZIP. |
| Steamworks | A integração do projeto principal permanece ativa. O AppID 480 pertence ao exemplo Spacewar. O mantenedor informou que não tem autorização da Valve. | Autorização para o uso e distribuição pretendidos continua sem comprovação. |

## Inspeção da cópia exata do ZIP

A cópia local de 9.508.638 bytes tem SHA-256 `ed8fc7fa160411aa209cedba0f512ec9a256021ed5a07fdcebdee3741aa7374e`, igual ao anexo original. A lista dos 15 arquivos confirma a ausência de CHD, BIOS e avisos de licença nesse ZIP. A extração foi feita numa pasta de auditoria.

- **Savestate:** SHA-256 `76247262bee61ecc3c7c9745ec65ed77ad48cbeb0e4256afeb11c5608141a3c8`, assinatura `RFFSST02`, cabeçalho de 32 bytes e payload de 8.974.441 bytes. A rotina de scan na árvore FBNeo local inclui RAM de programa 68K/Z80, tiles de sprites e texto e amostras ADPCM do Neo Geo CD. Os cinco nomes dessas regiões aparecem também na DLL exata da V6. Isso indica risco concreto de o estado carregar dados provenientes do disco. Falta confirmar a composição byte a byte e o direito de redistribuição. O arquivo `ngcd_rbffspec.fs` do ZIP tem 16.384 bytes, todos zero.
- **Banner:** a imagem do ZIP mostra personagens da franquia e o título do jogo. Foi localizada uma imagem de desenvolvimento, mas a cadeia de autoria e a permissão para uso público não foram estabelecidas.
- **Controles:** o arquivo do ZIP cita `mdqinc/SDL_GameControllerDB` no cabeçalho. SHA-256 `0f17831b6a9e0e9fc2f52bf8ab1bef1d30ffd307d966c2118e556db345ea7e96`. Falta identificar a revisão correspondente e incluir o aviso upstream.
- **Runtimes:** `msvcp140.dll`, `vcruntime140.dll` e `vcruntime140_1.dll` são versão 14.51.36247.0, com assinatura Microsoft válida. `SDL3.dll` informa versão 3.4.18.0. `steam_api64.dll` é versão 10.87.46.89, com assinatura Valve válida. O arquivo de configuração contém `480`. As assinaturas identificam os fornecedores dos binários. Os termos de redistribuição continuam dependendo da licença aplicável.
- **Backups de fontes:** `RealBout_V6_MSVC_Codigo_e_Support.zip` traz o código e o script de compilação do launcher, mas as DLL e EXE em Support têm hashes diferentes dos anexados à V6. O backup `RealBoutSpecial_NeoCD_Recreation.7z` também traz fontes do launcher. `RFFS-Online-v5-fontes.zip` contém o player e patches V5. Nenhum desses achados fecha a proveniência dos dois binários centrais da V6.

O ZIP da V6, o rascunho e o Steam multiplayer foram preservados. A auditoria avançou sobre a amostra exata, com pendências de procedência e permissões ainda abertas.

[Auditoria do ZIP](AUDITORIA_ZIP_ORIGINAL_V6.md) · [Proveniência](PROVENIENCIA_CODIGO_V6.md) · [Microsoft REDIST](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files) · [Banco de controles upstream](https://github.com/mdqinc/SDL_GameControllerDB/blob/master/LICENSE) · [Valve Spacewar](https://partner.steamgames.com/doc/sdk/api/example)

## Árvore de compilação original do núcleo localizada

A saída `NeoGeoCD/core-build/rffs_core.dll` tem o mesmo SHA-256 da DLL da Release, `61a02a4c4ee0150423b5a15180b1652c88793a006a3c8465fc819dce49bc7434`. O cache CMake aponta para a árvore `NeoGeoCD` adjacente e registra clang-cl, Ninja e configuração Release. O arquivo Ninja e o log da compilação estão presentes. A DLL e o log têm a mesma hora de modificação, 31/10/2026 às 20:49:49 UTC. A árvore FBNeo adjacente está no commit `9d82650bb9591966987a4c36ae8a1409ce0d5d68`. O diff dos seis arquivos modificados tem o mesmo identificador Git do patch já publicado no repositório privado, `c90934f7e6c5d14fb16dc3ec2d53e239e1131043`.

Esse achado vincula o núcleo da V6 à árvore de compilação e ao patch recuperado. Ainda não foi feita uma recompilação independente dessa árvore arquivada. O player com SHA-256 idêntico ao da Release foi localizado na pasta da instalação e em uma cópia do pacote, sem a respectiva árvore de compilação. O banco de controles da pasta de desenvolvimento tem SHA-256 idêntico ao do ZIP e indica o upstream no cabeçalho, mas a revisão upstream ainda não foi identificada. As autorizações relativas ao banner, savestate e Steamworks seguem sem comprovação.

## Proveniência do player — comparação dos executáveis

O EXE exato da V6 contém em texto os caminhos compilados de `ggpo/src/lib/ggpo/game_input.cpp` e `input_queue.cpp` da árvore original `NeoGeoCD`. A cópia de 1.106.944 bytes contém os caminhos equivalentes da árvore `NeoGeoCD-clean-20261102`. A seção de recursos é idêntica entre as duas cópias, mas as seções de código e dados diferem. A árvore original ainda tem `build_player.ps1` e `player/CMakeLists.txt`, que descrevem um build Release com clang-cl e Ninja. As cópias atuais de `rffs_player.cpp` e `rffs_net.cpp` dessa árvore registram modificação cerca de 2 min 36 s após a data do EXE preservado; o estado dos arquivos no momento da compilação ainda não foi demonstrado. Os caminhos compilados vinculam o GGPO ao diretório de trabalho sem provar a revisão de cada fonte do player.

O aviso upstream zlib do SDL_GameControllerDB foi incluído em `THIRD_PARTY_LICENSES/SDL_GameControllerDB-license.txt`. O Git blob do banco entregue é `e6c695e235230bc7eaa56799380a38fa726765fe`, distinto do master consultado. O ZIP original permanece intacto.
