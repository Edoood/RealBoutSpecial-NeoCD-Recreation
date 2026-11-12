# Revisão de licenças e redistribuição — v1.0.0

**Situação: pendências de conformidade. Este documento não constitui autorização legal nem certificação de distribuição.**

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
| `ngcd_rbffspec.fs` | Verificar origem e conteúdo do arquivo específico do jogo. |
| `gamecontrollerdb.txt` e arte/banner | Confirmar origem e preservar créditos/licenças aplicáveis. |
| ROM (CHD) e BIOS | Não constam do pacote analisado. |

## Ações pendentes para liberar uma publicação pública

1. Acrescentar **a licença integral do FBNeo dentro do ZIP da Release**; a cópia no repositório não basta para garantir que acompanhe o binário.
2. Acrescentar o código-fonte das modificações do emulador e instruções de compilação; o código C++ do launcher não substitui as fontes das outras DLLs e executáveis.
3. Verificar as condições de distribuição do Steamworks SDK e o uso do AppID de teste `480`.
4. Conferir os conteúdos do save state e do arquivo `.fs` para evitar distribuir recursos protegidos do jogo.
5. Conferir os demais avisos e condições de redistribuição das bibliotecas e do runtime Microsoft.

## Referências oficiais

- [Licença do FinalBurn Neo](https://github.com/finalburnneo/FBNeo/blob/master/src/license.txt)
- [Licença SDL](https://www.libsdl.org/license.php)
- [Microsoft — Redistributing Visual C++ Files](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files)
- [Steamworks](https://partner.steamgames.com/doc/sdk)

**Não há bloqueio técnico do GitHub à publicação de um arquivo com essas pendências, mas isso não significa que ele esteja regularizado para redistribuição.**
