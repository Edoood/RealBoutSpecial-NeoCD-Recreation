# Jurídico e licenças

Fúria Fatal RBS é um projeto comunitário não oficial para Windows. A referência a **Real Bout Fatal Fury Special (Neo Geo CD)** identifica o jogo usado com o projeto.

## Pacote Windows v1.0.1

O pacote contém o launcher, o núcleo de emulação, o player, o banner Fúria Fatal RBS e os componentes de execução. Cada jogador fornece sua própria imagem CHD e as BIOS `neocdz.zip` e `neogeo.zip`. O pacote vem sem savestate pré-carregado. Os fontes auditados e as instruções de compilação acompanham a release em um ZIP separado. [Release v1.0.1](../../releases/tag/v1.0.1)

## Licenças e redistribuição

| Componente | Avisos e condições |
| --- | --- |
| FinalBurn Neo | A licença integral acompanha o pacote. A auditoria vincula a DLL da V6 à árvore original e ao patch correspondente. A licença exige disponibilizar as alterações publicamente e restringe uso comercial e pedidos de doação. O repositório permanece privado para testes. |
| GGPO, Dear ImGui, SDL3 e SDL_GameControllerDB | Os avisos acompanham o ZIP Windows na pasta `Licencas` e estão disponíveis no repositório. |
| Runtime Microsoft Visual C++ | As três DLLs x64 conferidas são idênticas às cópias da pasta oficial de redistribuição do Visual Studio 2026 Community. A distribuição sem alterações está sujeita aos termos aplicáveis do Visual Studio. |
| Steamworks | A integração usa `steam_api64.dll` e o AppID 480, identificado pela Valve como o do exemplo Spacewar. A autorização específica para esse uso e distribuição não foi comprovada na auditoria. |

[Textos das licenças de terceiros](THIRD_PARTY_LICENSES/) · [Auditoria da V6](docs/VALIDACAO_PENDENCIAS_V6.md) · [Lista Microsoft REDIST](https://learn.microsoft.com/pt-br/visualstudio/releases/2026/redistribution) · [Valve Spacewar](https://partner.steamgames.com/doc/sdk/api/example)
