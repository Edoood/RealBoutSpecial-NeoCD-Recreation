# Auditoria do ZIP original da Release v1.0.0 — 12/11/2026

**Status: identidade do anexo confirmada; não liberado juridicamente para publicação.**

## Identidade e origem

- ZIP no GitHub (Release em rascunho): `Real.Bout.Special.NeoCD.Recreation.v1.0.zip`.
- Cópia inspecionada no Google Drive: `RealBoutSpecial_NeoCD_Recreation_Pacote_Funcional.zip`.
- Tamanho de ambos: **9.508.638 bytes**.
- SHA-256 calculado da cópia inspecionada: `ed8fc7fa160411aa209cedba0f512ec9a256021ed5a07fdcebdee3741aa7374e`.
- SHA-256 declarado pela API do GitHub para o anexo: `ed8fc7fa160411aa209cedba0f512ec9a256021ed5a07fdcebdee3741aa7374e`.
- **Conclusão: bytes idênticos; o inventário abaixo se aplica ao ZIP da Release.**
- O ZIP original, o rascunho e os binários foram preservados sem alterações.

## Inventário dos 15 arquivos do ZIP

Os tamanhos seguintes são **descompactados**, conforme o diretório central do ZIP.

| Arquivo | Bytes |
| --- | ---: |
| `RealBoutLauncher.exe` | 71.680 |
| `LEIA-ME.txt` | 1.491 |
| `Support/Jogar Real Bout Online.exe` | 1.106.432 |
| `Support/SDL3.dll` | 2.848.256 |
| `Support/steam_api64.dll` | 319.128 |
| `Support/steam_appid.txt` | 5 |
| `Support/banner.bmp` | 690.054 |
| `Support/msvcp140.dll` | 643.512 |
| `Support/vcruntime140.dll` | 178.616 |
| `Support/vcruntime140_1.dll` | 50.112 |
| `Support/rffs-runtime/rffs_wide_v5.dll` | 8.175.616 |
| `Support/rffs-runtime/settings.ini` | 951 |
| `Support/rffs-runtime/gamecontrollerdb.txt` | 610.126 |
| `Support/rffs-runtime/estado-inicial-protegido.rffsstate` | 8.974.473 |
| `Support/rffs-runtime/system/fbneo/ngcd_rbffspec.fs` | 16.384 |

## Resultados objetivos

1. **CHD/ROM/BIOS:** não há arquivo `.chd`, `.cue`, `.iso`, `neocdz.zip` ou `neogeo.zip` no ZIP. A ausência desses nomes não é uma auditoria de conteúdo binário interno, mas confirma a lista de arquivos distribuídos.
2. **Licenças:** não há nenhum arquivo identificado por `LICENSE`, `LICENCE`, `COPYING` ou `NOTICE` no ZIP. O texto completo da licença FBNeo existe no repositório em `THIRD_PARTY_LICENSES/FBNeo-license.txt`, porém **não acompanha este pacote ZIP**.
3. **Fontes correspondentes:** a V5 recuperada compila, mas não está comprovado que reproduz os binários desta V6. O código-fonte do launcher está no repositório.
4. **Save state:** `estado-inicial-protegido.rffsstate` com 8.974.473 bytes está incluído. Sua origem, conteúdo interno e permissões de redistribuição não foram estabelecidos nesta verificação de estrutura.
5. **Steamworks:** a presença de `steam_api64.dll` e `steam_appid.txt` exige verificar as condições de distribuição do SDK e do AppID antes de lançamento público.
6. **Divergência histórica:** o EXE `Support/Jogar Real Bout Online.exe` deste ZIP tem 1.106.432 bytes; a cópia funcional registrada anteriormente em um histórico local apresentava 1.192.960 bytes. **Isso prova diferença de tamanho, não falha de funcionamento**. Comparação individual de SHA-256 e testes funcionais ainda são necessários para declarar correspondência com aquela cópia.

## Ações para publicação

- Distribuir junto aos binários os avisos/licenças integrais aplicáveis, em especial a licença FBNeo e as demais condições das bibliotecas efetivamente incluídas. Isso **não ocorre no ZIP atual**.
- Disponibilizar o código-fonte/modificações correspondentes aos **binários efetivamente distribuídos**, e não apenas uma reconstrução aproximada da V5.
- Resolver a base de direitos para o estado de jogo; se ela não puder ser demonstrada, **não distribuir esse estado** sem autorização.
- Resolver termos e autorização específicos para biblioteca Steamworks/AppID 480, além dos runtimes Microsoft e demais arquivos de terceiros.
- Se desejado, preparar **uma nova cópia** de distribuição corrigida, preservando intocado o ZIP original como referência; não publicar automaticamente.
- Validar em Windows que o pacote final mantém as funcionalidades antes de substituir o anexo da Release.

**Este documento é uma auditoria técnica de arquivos e não constitui parecer jurídico, autorização ou aprovação de redistribuição.**
