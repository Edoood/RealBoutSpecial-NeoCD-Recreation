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

## Compilação isolada do candidato histórico do player

O backup `RFFS-Online-v5-fontes.zip` em disco contém versões anteriores às duas alterações na árvore original. O diff de `rffs_player.cpp` é uma linha no esvaziamento do buffer de áudio durante restauração. O diff de `rffs_net.cpp` é uma linha no cálculo do checksum do rollback. As datas e o conteúdo fazem desse backup um candidato à revisão usada pela V6.

Compilei os fontes do backup numa pasta isolada com o CMake dele, clang-cl, Ninja e cópias das dependências da árvore original. O build local completou os 30 passos e gerou 1.106.432 bytes, mesmo tamanho do original. As seções PE `.data`, `.tls` e `.rsrc` são idênticas. A seção `.text` tem a mesma extensão virtual, 735.158 bytes, com conteúdo diferente. A seção `.rdata` do build tem 16 bytes a mais antes do alinhamento. SHA-256 do build: `8e903ef9af54e4544e9abbaac918d508a8521948eafb2a2ccc88ac22548ca86c`. A compilação incluiu a integração Steam. A sessão Steam e a instalação funcional não foram usadas durante a compilação.

O resultado sustenta a hipótese de revisão histórica, mas não prova correspondência binária. A comparação ainda precisa controlar os caminhos de build e a versão exata da toolchain, e explicar as diferenças de código e dados. O ZIP original e a Release em rascunho permanecem intactos.

## Correspondência do player V6 confirmada por recompilação

Recompilei o candidato histórico em pasta isolada com clang-cl e Ninja. A única mudança na receita foi um mapeamento de prefixo de caminho (`-ffile-prefix-map`) na compilação do GGPO, para reproduzir os caminhos de diagnóstico da árvore histórica sem escrever nela. O fonte do player e a integração Steam ficaram como no backup. As sete seções PE do novo EXE são idênticas às da V6, incluindo código, dados, recursos e relocação. A comparação integral dos 1.106.432 bytes encontrou apenas três diferenças, nos offsets `0x80`, `0x81` e `0x82`, todos no campo COFF TimeDateStamp. Normalizando somente esse campo em memória, os arquivos são byte a byte iguais e o SHA-256 converge para `671768d621fe4ea13f55a9face89da4725343ea10600df7a6625fa73ada82cf6`. O hash do EXE recompilado sem normalização é `f77669bf928dd7c2fa094154366e59dc7bc837291a2e82d8d813f6b14cfa2e87`.

O backup local `RFFS-Online-v5-fontes.zip`, combinado com as dependências preservadas da árvore `NeoGeoCD`, reproduz o player da V6. Para disponibilizá-lo, ainda é preciso preparar a distribuição dos fontes correspondentes com as revisões dos insumos e avisos aplicáveis. Os fontes experimentais de `recovered-v5` incluem mudanças posteriores. A autorização de redistribuição do banner, savestate e Steamworks continua sendo uma questão separada. Nenhum arquivo da Release ou instalação funcional foi alterado.

## Validação funcional do savestate original

Executei o EXE e a DLL do ZIP autenticado em uma pasta isolada, com cópias locais de CHD, BIOS e savestate. O teste `--widescreen-test` retornou 0 e registrou `DIRECT_BOOT`, `MANUAL_STATE_AUTOLOAD_PASS` e `WIDESCREEN_TOGGLE_PASS 320 -> 426 -> 320, game RAM identical`. O estado continuou com SHA-256 `76247262bee61ecc3c7c9745ec65ed77ad48cbeb0e4256afeb11c5608141a3c8`. Isso confirma a compatibilidade e a integridade do arquivo com os binários e a mídia local testados.

O código do núcleo confirma que a serialização usa `ACB_FULLSCAN | ACB_READ`, sujeito a ajustes de contexto, e que a rotina Neo Geo CD inclui regiões de programa, tiles e ADPCM. A correspondência direta dos bytes do estado com arquivos do disco ainda não foi medida. O mantenedor informa que o banner foi criado para o projeto com o assistente e afirma que o savestate pode ser distribuído. Essas informações registram a origem declarada e a posição do mantenedor. O banner, o savestate e a integração Steam seguem no ZIP original, que não foi alterado.

## Comparação direta do savestate com o CHD

A comparação usou o savestate exato do ZIP original e a cópia local do CHD que passou no teste de carregamento. A leitura do CHD foi feita em modo somente leitura, descomprimindo os blocos em memória com libchdr. A busca encontrou cinco setores consecutivos de 2.048 bytes (índices 355–359 na sequência decodificada, dados a partir do byte 16 de cada frame) que coincidem integralmente, na mesma ordem e sem transformação, com os bytes 7.007.816–7.018.055 do payload do savestate (offsets a partir de zero, após o cabeçalho de 32 bytes). SHA-256 do primeiro setor coincidente: `358f1755208480b32bc0766ef5f444172b9c1c043fb97103d2fab37648d2f531`.

Isso confirma que o arquivo contém pelo menos 10.240 bytes copiados do disco. O teste identifica a origem desses bytes. A natureza exata dos dados, o total de bytes copiados e os direitos de redistribuição não foram determinados por esta amostra. Nenhum byte do CHD ou do savestate foi copiado para o Git. O ZIP e a Release em rascunho foram preservados.
