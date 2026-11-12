# Proveniência do código-fonte — Release funcional V6

**Resultado em 12/11/2026: código do launcher identificado; fontes do player e da DLL recuperados de V5; correspondência exata à V6 não comprovada.** Este documento não exige recompilar ou substituir a V6 funcional.

## Evidências verificadas

| Componente incluído no ZIP V6 | Hash SHA-256 auditado | Código disponível | Conclusão |
| --- | --- | --- | --- |
| `RealBoutLauncher.exe` | `11550860d281c877ed73e0689e87ce9c04313a497ac96de7526d4b5dbefc822e` | [`src/RealBoutLauncher.cpp`](../src/RealBoutLauncher.cpp) e workflow de compilação do launcher | Código disponível; reprodução bit a bit do executável da Release não comprovada |
| `Support/Jogar Real Bout Online.exe` | `671768d621fe4ea13f55a9face89da4725343ea10600df7a6625fa73ada82cf6` | [`recovered-v5/rffs_player.cpp`](../recovered-v5/rffs_player.cpp), `rffs_net.cpp`, `rffs_steam.cpp`, `rffs_gl.cpp`, `rffs_menu.cpp`, `rffs_input_bridge.cpp` e cabeçalhos correspondentes | Fontes recuperadas do histórico V5; correspondência com a V6 não demonstrada |
| `Support/rffs-runtime/rffs_wide_v5.dll` | `61a02a4c4ee0150423b5a15180b1652c88793a006a3c8465fc819dce49bc7434` | [`recovered-v5/FBNeo-widescreen.patch`](../recovered-v5/FBNeo-widescreen.patch) sobre FBNeo commit `9d82650bb9591966987a4c36ae8a1409ce0d5d68` | Patch disponível e build experimental funcional para compilação; equivalência com a DLL V6 não demonstrada |

A pasta [`recovered-v5`](../recovered-v5/README.md) também inclui `GGPO-Steam.patch`, `configure_core.py`, `prepare-dependencies.ps1` e `player/CMakeLists.txt` reconstruído. Referências históricas: GGPO `7ddadef8546a7d99ff0b3530c6056bc8ee4b9c0a`, Dear ImGui `6d910d5487d11ca567b61c7824b0c78c569d62f0`, SDL3 3.4.18.

## O que os testes já demonstraram

- GitHub Actions [43140522509](https://github.com/Edoood/RealBoutSpecial-NeoCD-Recreation/actions/runs/43140522509) compilou uma versão experimental das fontes recuperadas.
- GitHub Actions [43142102098](https://github.com/Edoood/RealBoutSpecial-NeoCD-Recreation/actions/runs/43142102098) verificou exportações necessárias da DLL experimental.
- Nenhum dos dois substituiu a V6 oficial. Os testes acima **não demonstram** que os fontes recuperados são os usados para gerar os binários V6, nem que o save state e o Netplay serão compatíveis com um novo build.
- O ZIP original auditado e a cópia com licenças anexada à Release rascunho contêm os **mesmos arquivos binários do jogo**, e o script de preparação verificou a identidade de conteúdo dos 15 arquivos originais.

## Pendência exata (sem repetir auditoria geral)

**Falta identificar o arquivo de desenvolvimento / commit / conjunto de fontes que produziu o `Jogar Real Bout Online.exe` e a `rffs_wide_v5.dll` da V6.** Esses fontes podem existir em outro backup do mantenedor; não foram encontrados entre os arquivos atualmente identificados neste repositório. A V5 recuperada ajuda na reconstrução, mas não deve ser rotulada como "fontes V6 correspondentes" sem prova.

Para cumprir a obrigação de publicar alterações ao FBNeo, o conteúdo disponibilizado deve ser o efetivamente correspondente ao binário distribuído. Não exigir uma comparação de hash entre EXE recompilado e original se há diferenças legítimas de toolchain; exigir **correspondência real das alterações e dos arquivos-fonte**, comprovada pela cadeia de desenvolvimento ou por verificação técnica específica.

**Próxima ação quando disponível uma cópia da pasta fonte V6:** comparar os arquivos e patches com a versão recuperada e registrar as mudanças; não alterar o executável nem a DLL da Release sem testes.

## Não misturar as pendências

A presença de licenças na **nova cópia** já foi resolvida, conforme [revisão de licenças](REVISAO_LICENCAS.md). Este documento trata **apenas da proveniência dos fontes da V6**, não reabre a verificação de licenças ou do save state.

## Comparação estática direta entre DLL original da V6 e DLL reconstruída

**Executada com sucesso em 12/11/2026**, [GitHub Actions 43154546076](https://github.com/Edoood/RealBoutSpecial-NeoCD-Recreation/actions/runs/43154546076). O workflow [`compare-v6-core.yml`](../.github/workflows/compare-v6-core.yml) obtém o ZIP **original** de uma Release em rascunho, confirma o SHA-256 original, baixa o binário experimental previamente compilado e compara as exportações com `dumpbin`, sem alterar nem substituir nenhum dos dois ZIPs da Release.

| Medida | DLL V6 distribuída | DLL V5 reconstruída |
| --- | ---: | ---: |
| Tamanho | 8.175.616 bytes | 8.054.272 bytes |
| Número de exportações | 2.834 | 2.835 |
| Símbolos de exportação em comum | 2.825 | 2.825 |
| Exportações exclusivas desta DLL | 9 | 10 |

- SHA-256 V6: `61a02a4c4ee0150423b5a15180b1652c88793a006a3c8465fc819dce49bc7434`.
- SHA-256 V5 reconstruída: `b84dfab5792d51b2c3c46ceeeb19729344dfd35ed770d40bcaa2b49690bcfc6d`.
- **Oito das nove exportações ausentes na reconstrução têm nomes decorados de rotinas internas de STL/C++** (`vector`, `string`, `ostringstream`), possivelmente influenciados pela versão do compilador ou pelas otimizações.
- **A nona exportação da V6 não presente na reconstrução é específica do widescreen:** `?RffsWideSprites@@3PAURffsWideSprite@@A` (variável C++ `RffsWideSprites`). O nome não foi encontrado nos arquivos `recovered-v5/FBNeo-widescreen.patch` nem `recovered-v5/rffs_player.cpp`. Portanto, pode haver alterações de código/fonte da V6 ainda não recuperadas.
- A reconstrução apresenta dez exportações exclusivas, todas identificadas com nomes de rotinas de biblioteca padrão C++ (especialmente `vector` e `ostringstream`).
- **Conclusão rigorosa:** as interfaces exportadas têm sobreposição elevada, mas não são idênticas. Essa semelhança **não demonstra o código-fonte correspondente à V6 nem a equivalência de comportamento**. Adicionar um símbolo inventado só para igualar a lista seria incorreto e pode causar erro de memória.
- A reconstrução da V5 já tinha sido compilada e verificada quanto aos 20 símbolos essenciais esperados pelo player recuperado; isso é um resultado separado de compatibilidade estática e não garante o funcionamento da V6.
- **Próximo trabalho técnico específico:** localizar a definição original de `RffsWideSprites` (e seu tipo/uso) no código que gerou a DLL V6, em vez de repetir a compilação ou substituir a DLL funcional. Se não houver fonte, pode-se comparar outras evidências de comportamento sem apresentar a reconstrução como exata.

**O ZIP original e a cópia com licenças estão preservados. Não foi publicada uma nova DLL.**

## Fonte widescreen adicional recuperada em 12/11/2026

O mantenedor recuperou dos arquivos locais do Codex o arquivo `revisao-widescreen-atual.patch`, preservado no repositório como [`recovered-v5/FBNeo-widescreen-v6-revision.patch`](../recovered-v5/FBNeo-widescreen-v6-revision.patch). Ele contém a **definição e utilização real** de `RffsWideSprite RffsWideSprites[384]`, símbolo que estava ausente do patch V5 e era exportado pela DLL V6 funcional. Também amplia o tratamento de câmera, camadas e atores fora da área original de 320 pixels.

O build separado [Experimental V6 widescreen revision build](../.github/workflows/build-widescreen-revision.yml) aplica **essa versão recuperada do patch apenas numa cópia temporária do ambiente GitHub Actions** e compila um novo artefato experimental. O patch V5 original e todos os arquivos da Release V6 permanecem intocados.

**Limite da conclusão:** encontrar o símbolo no fonte recuperado é evidência concreta da origem da funcionalidade, mas **ainda não prova que o patch é integralmente correspondente à DLL V6** nem garante que a compilação experimental seja compatível com o save state. Não substituir DLL/EXE da V6.

## Resultado da recompilação com o patch recuperado (12/11/2026)

**O workflow de compilação [43160858588](https://github.com/Edoood/RealBoutSpecial-NeoCD-Recreation/actions/runs/43160858588) terminou com sucesso.** Os fontes com `RffsWideSprites` foram aplicados a uma árvore FBNeo isolada e a nova DLL experimental foi compilada. O ZIP da V6 e os arquivos de Release não foram alterados.

A comparação direta com a DLL da V6 também foi concluída com sucesso em [GitHub Actions 43229497860](https://github.com/Edoood/RealBoutSpecial-NeoCD-Recreation/actions/runs/43229497860):

| Métrica | V6 original | DLL recompilada com patch recuperado |
| --- | ---: | ---: |
| Exportações | 2.834 | 2.836 |
| Exportações compartilhadas | 2.826 | 2.826 |
| Exportações exclusivas | 8 | 10 |
| Tamanho (bytes) | 8.175.616 | 8.077.824 |
| SHA-256 | `61a02a4c4ee0150423b5a15180b1652c88793a006a3c8465fc819dce49bc7434` | `fbbc3a38abb83fcf6d2e883b89797f00fc437327e18a59332a662d3e2008d5a8` |

**Diferença eliminada:** o símbolo C++ `?RffsWideSprites@@3PAURffsWideSprite@@A`, que antes era exclusivo da DLL V6, agora aparece na DLL reconstruída. As oito exportações ainda exclusivas da V6 possuem nomes decorados de rotinas internas STL/C++; não se pode deduzir delas alterações na lógica do jogo. Esse resultado não demonstra correspondência integral dos fontes nem funcionalidade de save state ou Netplay. A DLL recompilada permanece isolada, e não deve ser substituída no pacote funcional.

## Teste real de carregamento da DLL experimental

Em 12/11/2026, a execução isolada [GitHub Actions 43237225604](https://github.com/Edoood/RealBoutSpecial-NeoCD-Recreation/actions/runs/43237225604) recompilou o núcleo com o patch widescreen recuperado e passou em um **teste real de carregamento no Windows**, implementado em [`recovered-v5/smoke_core.py`](../recovered-v5/smoke_core.py).

O runner carregou `rffs_core.dll` com `ctypes.CDLL`, chamou `retro_api_version()` (retorno **1**) e `retro_get_system_info()` (nome **FinalBurn Neo (neogeo subset)**, versão **v1.0.0.03**, extensões **zip|7z|cue|ccd|chd**, `need_fullpath=true`). Resultado registrado: **PASS**. Nenhum arquivo de jogo, ROM, BIOS, CHD, Steamworks ou estado protegido foi utilizado.

### Estado das pendências

| Verificação | Situação | Evidência / limite |
| --- | --- | --- |
| Origem do símbolo `RffsWideSprites` | **Resolvida** | Patch recuperado nos arquivos do Codex e preservado no repositório |
| Recompilar com o patch recuperado | **Resolvida** | Workflow de build 43160858588 e 43237225604 |
| Exportação de `RffsWideSprites` na DLL nova | **Resolvida** | Comparação estática 43229497860; exportação agora comum à V6 |
| Carregamento no Windows e API básica | **Resolvida** | Smoke test 43237225604, sem conteúdo de jogo |
| Equivalência binária da DLL original V6 | **Não comprovada** | Tamanho e SHA256 diferentes; exportações restantes também diferentes |
| Reprodução visual dos cenários widescreen | **Não comprovada no build reconstruído** | Requer testes funcionais e comparação de frames no jogo |
| Continuidade de câmera, atores, HUD e transições em diferentes fases | **Não comprovada no build reconstruído** | Requer execução e capturas de múltiplos cenários |
| Compatibilidade do savestate protegido da V6 | **Não comprovada; possível incompatibilidade** | Identificador de compatibilidade depende dos bytes da DLL |
| Compatibilidade de áudio e Netplay | **Não testada** | Não faz parte dos testes de estrutura; não alterar o sistema multiplayer funcional |

**Política de segurança:** não substituir os binários V6 da Release, não trocar a DLL em instalações funcionais e não modificar o save state protegido. O pacote experimental é apenas um artefato separado, sem jogo/BIOS/save.
