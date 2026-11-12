# Revisão de licenças e redistribuição — v1.0.0

**Situação: fontes V5 recuperadas; conformidade da distribuição binária ainda não concluída. Este documento não constitui autorização legal nem certificação de distribuição.**

Este documento acompanha o projeto para registrar as condições que devem ser atendidas antes de uma publicação pública do pacote binário. A licença oficial do FinalBurn Neo está arquivada em [THIRD_PARTY_LICENSES/FBNeo-license.txt](../THIRD_PARTY_LICENSES/FBNeo-license.txt).

## Componentes

| Arquivo/componente | Origem / obrigação a conferir |
| --- | --- |
| `RealBoutLauncher.exe` | Launcher próprio; código-fonte em `src/RealBoutLauncher.cpp`. |
| `Jogar Real Bout Online.exe` | Executável do projeto; disponibilizar o código-fonte completo e identificar dependências de terceiros. |
| `rffs_wide_v5.dll` | DLL modificada com referências a FBNeo; disponibilizar código-fonte correspondente e respeitar licença FBNeo. |
| `SDL3.dll` | Biblioteca SDL, geralmente licenciada sob zlib; conservar avisos pertinentes. |
| `steam_api64.dll` | Biblioteca proprietária Steamworks; verificar permissão de distribuição nos termos do SDK. |
| `steam_appid.txt` | AppID 480 é o aplicativo de exemplo Spacewar, não um AppID de produção do projeto. |
| `vcruntime140.dll`, `vcruntime140_1.dll`, `msvcp140.dll` | Verificar termos de redistribuição Microsoft Visual C++. |
| `estado-inicial-protegido.rffsstate` | Save state de ~9 MB; ainda não foi comprovado que não contém material protegido do jogo. |
| `ngcd_rbffspec.fs` | Amostra fornecida pelo mantenedor: 16 KB preenchidos por zeros; sem dados de jogo identificados nessa amostra. |
| `gamecontrollerdb.txt` e arte/banner | Confirmar origem e preservar créditos/licenças aplicáveis. |
| ROM (CHD) e BIOS | Não constam do pacote analisado. |

## Ações pendentes para liberar uma publicação pública

1. Acrescentar **a licença integral do FBNeo dentro do ZIP da Release**; a cópia no repositório não basta para garantir que acompanhe o binário.
2. **Parcialmente atendido:** patches FBNeo/GGPO e fontes V5 recuperados em [`recovered-v5/`](../recovered-v5/README.md); `player/CMakeLists.txt` reconstruído, build Windows experimental e verificação estática de exportações aprovados no [GitHub Actions](https://github.com/Edoood/RealBoutSpecial-NeoCD-Recreation/actions/runs/43142102098). **Ainda falta demonstrar correspondência dos fontes com os binários V6 distribuídos**, não apenas compilar uma V5 experimental.
3. Verificar as condições de distribuição do Steamworks SDK e o uso do AppID de teste `480`.
4. Conferir o conteúdo do save state para evitar distribuir recursos protegidos do jogo. O arquivo `.fs` fornecido separadamente foi examinado e continha 16 KB de zeros.
5. Conferir os demais avisos e condições de redistribuição das bibliotecas e do runtime Microsoft.

## Referências oficiais

- [Licença do FinalBurn Neo](https://github.com/finalburnneo/FBNeo/blob/master/src/license.txt)
- [Licença SDL](https://www.libsdl.org/license.php)
- [Microsoft — Redistributing Visual C++ Files](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files)
- [Steamworks](https://partner.steamgames.com/doc/sdk)

**Não há bloqueio técnico do GitHub à publicação de um arquivo com essas pendências, mas isso não significa que ele esteja regularizado para redistribuição.**

## Progresso documentado

- Licenças completas FBNeo, GGPO, ImGui e SDL3 disponíveis na pasta [`THIRD_PARTY_LICENSES`](../THIRD_PARTY_LICENSES/).
- Código recuperado e documentação de reconstrução histórica em [`recovered-v5/README.md`](../recovered-v5/README.md).
- **A inclusão desses documentos no repositório não os acrescenta automaticamente ao ZIP já anexado à Release.** A distribuição final deve acompanhá-los.

## Verificação do rascunho da Release v1.0.0

- Release `v1.0.0` permanece **rascunho**, com `Real.Bout.Special.NeoCD.Recreation.v1.0.zip` anexado (9.508.638 bytes; SHA-256 do anexo conforme API GitHub: `ed8fc7fa160411aa209cedba0f512ec9a256021ed5a07fdcebdee3741aa7374e`).
- O texto de apresentação em português e inglês já existe e descreve corretamente a emulação, os recursos, a instalação e que CHD/BIOS não estão incluídos.
- O conteúdo interno desse ZIP **não foi inspecionado neste procedimento**. Não se pode afirmar que contém todas as licenças, nem que o save state não contém dados protegidos.
- A compilação da V5 experimental e seu relatório de exportações **não validam a correspondência** com os binários V6. Nada disso exige modificar ou substituir o anexo.
- **Antes de tornar a Release pública:** conferir os arquivos de licença efetivamente presentes no ZIP, o código-fonte correspondente à V6, os termos dos componentes redistribuídos e o conteúdo/origem do estado inicial protegido. A situação atual não é certificação legal de distribuição.
