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
| `steam_api64.dll` | A documentação Valve admite redistribuir a DLL com jogos Steamworks; é necessário confirmar que a autorização/termos contratuais se aplicam a este projeto. Permissão sobre a DLL não implica permissão para usar o AppID 480 em publicação. |
| `steam_appid.txt` | AppID 480 é o aplicativo de exemplo Spacewar, não um AppID de produção do projeto. |
| `vcruntime140.dll`, `vcruntime140_1.dll`, `msvcp140.dll` | A Microsoft condiciona a distribuição desses binários a licenças do Visual Studio e lista REDIST. Conferir se as cópias e sua distribuição estão cobertas; alternativa futura é instalação do pacote oficial VC++ Redistributable, se compatível, sem tocar na V6 original. |
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

## Critérios objetivos para publicação da V6 original

A **V6 funcional permanece a versão oficial**. A Release v1.0.0 deve permanecer em **rascunho** até que as verificações abaixo sejam documentadas. Recompilar a V5 experimental não é condição para lançar nem comprovação de conformidade da V6.

| Ponto | Evidência disponível | Condição para aprovar |
| --- | --- | --- |
| Imagem CHD e BIOS | README e manifesto informam que não acompanham a distribuição; ZIP não foi aberto nesta auditoria. | Conferir a lista interna do anexo e confirmar ausência de arquivos do jogo/BIOS sem autorização. |
| Licença FBNeo | Texto completo está em `THIRD_PARTY_LICENSES/FBNeo-license.txt` no repositório. | Conferir se o texto integral acompanha efetivamente o binário no material distribuído, além dos avisos das outras bibliotecas. |
| Código-fonte da V6 | Código do launcher em `src/`; fontes e patches **V5** em `recovered-v5/` compilam; equivalência com os binários **V6** ainda não demonstrada. | Identificar, publicar e associar à versão os fontes e alterações que geraram os binários distribuídos; os patches V5 não bastam sem essa correspondência. |
| Save state protegido | `estado-inicial-protegido.rffsstate` consta do manifesto, mas seu conteúdo e as permissões de redistribuição não foram validados. | Examinar o conteúdo e obter fundamento para sua redistribuição, especialmente caso inclua dados protegidos do jogo. |
| Steamworks | `steam_api64.dll` e `steam_appid.txt` são citados no manifesto; o AppID 480 pertence ao aplicativo de exemplo Spacewar. | Confirmar o direito de redistribuir a biblioteca conforme os termos aplicáveis e a autorização/condições de uso do AppID em uma versão pública. Não presumir autorização pelo simples funcionamento. |
| Outros componentes | SDL, GGPO, ImGui, runtime Visual C++, arquivo de controle e arte foram identificados. | Documentar procedência e anexar os avisos/licenças aplicáveis a cada componente efetivamente distribuído. |

**Status: ainda não certificado para publicação pública.** Isso é uma lista de verificações, não um parecer jurídico. Os resultados positivos de compilação ou testes técnicos não substituem essas verificações. Não alterar o ZIP da Release nem publicar o rascunho automaticamente.

### Fontes primárias consultadas

- [FBNeo — licença oficial](https://github.com/finalburnneo/FBNeo/blob/master/src/license.txt): inclui exigência de publicar modificações, licença integral e restrições de uso comercial/doações.
- [Valve — exemplo Steamworks/Spacewar](https://partner.steamgames.com/doc/sdk/api/example): identifica o AppID 480 como exemplo e informa condições de distribuição das bibliotecas para jogos Steamworks. Esse texto não concede autorização genérica para reutilizar o identificador 480 em qualquer lançamento.
- [Valve — documentação do SDK](https://partner.steamgames.com/doc/sdk): verificar condições contratuais concretas para a distribuição da biblioteca proprietária.

## Resultado da inspeção exata do ZIP

O ZIP da Release foi comparado com a cópia funcional do Google Drive por **SHA-256 idêntico**. A inspeção confirmou 15 arquivos, nenhum arquivo de licença/aviso separado no pacote, nenhuma ROM/BIOS com os nomes conhecidos e presença do estado inicial protegido. Há divergência de tamanho entre o executável do anexo (1.106.432 bytes) e outra cópia funcional registrada no histórico (1.192.960 bytes); a equivalência de binários ainda requer verificação. Relatório integral: [AUDITORIA_ZIP_ORIGINAL_V6.md](AUDITORIA_ZIP_ORIGINAL_V6.md).

**A inclusão de licenças no repositório não corrige sua ausência no ZIP que está anexado.** Para manter a V6 original intacta, quaisquer mudanças no pacote devem ocorrer apenas em uma cópia separada, após a verificação dos direitos sobre o estado, do código-fonte correspondente e das demais dependências.

## Esclarecimentos de fontes oficiais (12/11/2026)

- A [documentação oficial do Steamworks Spacewar](https://partner.steamgames.com/doc/sdk/api/example?l=brazilian) identifica **480 como o AppID do aplicativo de exemplo**, e afirma que `steam_api.dll` pode acompanhar jogos Steamworks. **Não afirma que 480 pode ser reutilizado livremente em produtos públicos de terceiros**. A situação do projeto requer validação dos termos aplicáveis, sem necessidade de alterar o jogo durante a análise.
- A [Microsoft](https://learn.microsoft.com/pt-br/cpp/windows/redistributing-visual-cpp-files) afirma que redistribuição de runtime VC++ está sujeita aos termos de licença do Visual Studio e a listas REDIST. É preciso confirmar os direitos de redistribuição das DLLs presentes no ZIP. A [alternativa oficial de redistributable x64](https://learn.microsoft.com/cpp/windows/latest-supported-vc-redist) pode ser avaliada para um **pacote futuro** após teste; isso não altera o ZIP V6 atual.
- A [licença oficial FBNeo](https://github.com/finalburnneo/FBNeo/blob/master/src/license.txt) exige a licença integral, publicação das modificações e uso não comercial, entre outras condições. A licença existente no repositório deve ser conferida quanto à integridade e incluída no material efetivamente distribuído. A cópia dentro do repositório não modifica o ZIP anexado.

### Decisão de manutenção

Manter o **ZIP original da V6 como referência intocada** e **não declarar a Release pronta** enquanto faltarem a licença no pacote distribuído, fontes correspondentes à V6, avaliação do save state e confirmação de direitos sobre os componentes redistribuídos. Uma eventual cópia corrigida de distribuição, se desejada, é um novo arquivo, não uma substituição silenciosa do pacote funcional.
