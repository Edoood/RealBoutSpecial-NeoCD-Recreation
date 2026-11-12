# Revisão de licenças e redistribuição — v1.0.0

**Situação: fontes V5 recuperadas; conformidade da distribuição binária ainda não concluída. Este documento não constitui autorização legal nem certificação de distribuição.**

**Licenças no novo ZIP: CONCLUÍDO.** Permanecem questões separadas sobre fontes V6 e direitos de alguns componentes. A Release segue em rascunho.

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

1. **RESOLVIDO NA CÓPIA COM LICENÇAS:** o segundo ZIP anexado à Release em rascunho inclui a licença integral FBNeo, junto com GGPO, SDL3, ImGui e um aviso de distribuição. O ZIP original permanece inalterado e sem esses avisos.
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
- **Atualizado:** uma cópia SEPARADA com as quatro licenças e um aviso já foi criada e anexada à Release em rascunho. O primeiro ZIP original permanece inalterado.

## Verificação do rascunho da Release v1.0.0

- Release `v1.0.0` permanece **rascunho**, com `Real.Bout.Special.NeoCD.Recreation.v1.0.zip` anexado (9.508.638 bytes; SHA-256 do anexo conforme API GitHub: `ed8fc7fa160411aa209cedba0f512ec9a256021ed5a07fdcebdee3741aa7374e`).
- O texto de apresentação em português e inglês já existe e descreve corretamente a emulação, os recursos, a instalação e que CHD/BIOS não estão incluídos.
- O conteúdo interno desse ZIP **não foi inspecionado neste procedimento**. Não se pode afirmar que contém todas as licenças, nem que o save state não contém dados protegidos.
- A compilação da V5 experimental e seu relatório de exportações **não validam a correspondência** com os binários V6. Nada disso exige modificar ou substituir o anexo.
- **Antes de tornar a Release pública:** usar como referência a **cópia com licenças**, cujos arquivos originais tiveram hashes preservados no GitHub Actions, e resolver as pendências reais sobre fontes V6 e termos dos componentes; o save state não requer autorização automática por ser um estado salvo. A situação atual não é certificação legal de distribuição.

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

**Atualização:** o primeiro ZIP permanece sem arquivos de licença; o segundo ZIP, anexado à mesma Release em rascunho, já inclui FBNeo, GGPO, SDL3 e ImGui, além de aviso. Os direitos relativos às demais dependências ainda precisam de confirmação.

## Esclarecimentos de fontes oficiais (12/11/2026)

- A [documentação oficial do Steamworks Spacewar](https://partner.steamgames.com/doc/sdk/api/example?l=brazilian) identifica **480 como o AppID do aplicativo de exemplo**, e afirma que `steam_api.dll` pode acompanhar jogos Steamworks. **Não afirma que 480 pode ser reutilizado livremente em produtos públicos de terceiros**. A situação do projeto requer validação dos termos aplicáveis, sem necessidade de alterar o jogo durante a análise.
- A [Microsoft](https://learn.microsoft.com/pt-br/cpp/windows/redistributing-visual-cpp-files) afirma que redistribuição de runtime VC++ está sujeita aos termos de licença do Visual Studio e a listas REDIST. É preciso confirmar os direitos de redistribuição das DLLs presentes no ZIP. A [alternativa oficial de redistributable x64](https://learn.microsoft.com/cpp/windows/latest-supported-vc-redist) pode ser avaliada para um **pacote futuro** após teste; isso não altera o ZIP V6 atual.
- A [licença oficial FBNeo](https://github.com/finalburnneo/FBNeo/blob/master/src/license.txt) exige a licença integral, publicação das modificações e uso não comercial, entre outras condições. A licença existente no repositório deve ser conferida quanto à integridade e incluída no material efetivamente distribuído. A cópia dentro do repositório não modifica o ZIP anexado.

### Decisão de manutenção

Manter o **ZIP original da V6 como referência intocada**. A **cópia com licenças foi gerada e anexada ao rascunho**. Não declarar liberação jurídica enquanto faltarem fontes correspondentes à V6 e confirmação das condições dos demais componentes. Save state não requer autorização automaticamente; questões específicas dependem de seu conteúdo.

## Decisão prática de publicação — pendências reais (12/11/2026)

**A V6 funcional continua sendo a referência. Não é preciso recompilar o jogo novamente, nem obter autorização genérica para criar ou salvar estados do emulador.** Também não se deve declarar o ZIP atual regularizado: ele foi auditado, mas faltam passos específicos.

1. **RESOLVIDO NA CÓPIA COM LICENÇAS — texto FBNeo e avisos de terceiros.** O segundo ZIP foi criado e anexado à Release rascunho, com FBNeo, GGPO, SDL3, ImGui e aviso de distribuição. O primeiro ZIP original permanece inalterado. Isto resolve a **presença desses textos** no pacote novo, mas não a obrigação separada de disponibilizar as modificações correspondentes à V6.
2. **Obrigação objetiva — código-fonte correspondente à V6.** A V5 recuperada compila, mas **não é prova de correspondência** com o binário V6. Localizar/publicar o código ou patches efetivamente usados no executável e na DLL distribuídos, com referência clara à versão; não exigir binários idênticos bit a bit se o ambiente de compilação diferir, mas demonstrar que os fontes são realmente correspondentes.
3. **Termos específicos — Steamworks.** A Valve permite redistribuir a biblioteca `steam_api` com jogos Steamworks em condições próprias. O AppID `480` é do aplicativo de exemplo Spacewar; seu uso no nosso projeto **não é automaticamente autorizado** por essa informação. Confirmar autorização aplicável ao lançamento antes de afirmar conformidade. Não alterar o funcionamento do Netplay para realizar essa conferência.
4. **Redistribuição Microsoft.** As DLLs de runtime VC++ podem ser redistribuídas dentro das condições de licença/REDIST do Visual Studio. Confirmar procedência; um pacote oficial VC++ Redistributable pode ser empregado numa nova distribuição se for necessário e testado.
5. **Save state — sem autorização automática presumida.** Um arquivo `.rffsstate` não demanda, apenas por ser um save state, uma autorização específica da desenvolvedora do jogo. **Só há uma questão de direitos se o conteúdo reproduzir material protegido de terceiros em grau juridicamente relevante.** A inspeção anterior comprovou presença, tamanho e estrutura, **não comprovou infração** e também não examinou o conteúdo integral; não manter a expressão genérica “obter autorização para save state” como bloqueio independente. Se o arquivo contiver dados protegidos que não possam ser distribuídos, aí será preciso avaliar uma alternativa ou a autorização cabível.
6. **Outros materiais.** Confirmar a procedência/licenças do banner e do arquivo de mapeamento de controles efetivamente usados.

**Resumo operacional:** não repetir auditorias gerais nem substituir o EXE funcional. **A cópia do ZIP com os avisos já está pronta**; antes de publicar, resolver as condições de redistribuição e a correspondência dos fontes V6, além de testar o pacote em Windows. O anexo atual da Release é um rascunho; sua publicação ainda não foi autorizada nem executada. Este documento é uma revisão prática, não parecer jurídico.

## Preparação de cópia separada com licenças

Foi incluído o utilitário [`scripts/prepare-v6-license-copy.py`](../scripts/prepare-v6-license-copy.py), que recebe o **ZIP original auditado**, exige o SHA-256 exato, verifica os hashes dos principais binários e do save state, copia todas as 15 entradas originais **sem modificar o conteúdo** e adiciona em um **novo ZIP** os textos disponíveis de FBNeo, GGPO, Dear ImGui e SDL3, com `AVISOS_DE_DISTRIBUICAO.txt`. Se o novo arquivo já existir, não o sobrescreve. Não publica nem altera a Release.

**Concluído:** o script foi executado pelo GitHub Actions [run 43152764479](https://github.com/Edoood/RealBoutSpecial-NeoCD-Recreation/actions/runs/43152764479) com sucesso. A cópia nova foi gerada e anexada à Release como `Real.Bout.Special.NeoCD.Recreation.v1.0-LICENCAS-COPIA-NAO-PUBLICAR.zip` (9.537.637 bytes; SHA-256 `ba55e9700afa590baaaa7c68584aaa00f2dc888b3499835e9e3650e6c3e4f4e2`). O ZIP original permanece anexado separadamente, com o mesmo hash original. A inclusão das licenças não prova as permissões dos demais componentes; o rascunho não foi publicado.

## Diagnóstico específico de fontes V6

A localização dos fontes foi verificada separadamente em [PROVENIENCIA_CODIGO_V6.md](PROVENIENCIA_CODIGO_V6.md), associando os hashes auditados do launcher, executável e DLL aos arquivos-fonte realmente disponíveis. O código do launcher foi identificado, e os fontes do player e patches estão documentados como **V5 recuperada**; ainda **não foi comprovada a correspondência destes últimos com os binários V6**. Esta pendência não foi encerrada artificialmente nem depende de repetir compilação sem nova evidência.
