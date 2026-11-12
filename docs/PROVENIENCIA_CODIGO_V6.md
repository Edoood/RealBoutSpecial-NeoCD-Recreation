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
