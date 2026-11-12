# Real Bout Special NeoCD Recreation

Projeto comunitário para **Windows** baseado na **emulação da versão Neo Geo CD de Real Bout Fatal Fury Special**.

> **Não é uma recompilação nativa do jogo.** O projeto utiliza emulação.

## Principais diferenciais

- **Widescreen**.
- **Músicas da versão Neo Geo CD, com áudio de CD**.
- **Netplay via Steam** para partidas online.
- **Placar automático**.
- **Personagens EX desbloqueados** mediante estado de jogo compatível.
- **Launcher gráfico para Windows**, com seleção de ROM e BIOS e botão PLAY.
- Menu de opções acessível pela tecla **F1**.

## Requisitos

- Windows.
- Steam em execução para os recursos de Netplay.
- Imagem **CHD** compatível do jogo, fornecida pelo usuário.
- BIOS **neocdz.zip** e **neogeo.zip**, fornecidas pelo usuário.
- Componentes de execução compatíveis, conforme a distribuição autorizada.

**ROM/CHD e BIOS não são fornecidas neste repositório.** Use apenas arquivos que você tenha direito de utilizar.

## Instalação e execução

1. Obtenha uma distribuição do launcher e seus componentes autorizados.
2. Extraia o pacote completo, preservando a estrutura de pastas.
3. Execute `RealBoutLauncher.exe`.
4. Selecione a imagem CHD e as duas BIOS solicitadas.
5. Clique em **PLAY**.
6. Para acessar opções dentro do jogo, pressione **F1**. Para jogar online, mantenha a Steam aberta.

## Distribuição e código-fonte

O repositório contém o [código-fonte do launcher](src/RealBoutLauncher.cpp), [código e patches recuperados da V5](recovered-v5/README.md), documentação e [licenças de terceiros](THIRD_PARTY_LICENSES/). O projeto utiliza emulação; o launcher não recompila nativamente o jogo original.

**Importante:** as fontes recuperadas da V5 já foram compiladas experimentalmente, mas **não foi demonstrado que correspondem exatamente ao executável e à DLL da V6 funcional**. Uma compilação bem-sucedida não equivale a comprovação dos direitos de redistribuição.

O ZIP da Release `v1.0.0` permanece em **rascunho** e não deve ser considerado uma distribuição aprovada. O [inventário auditado do pacote](docs/AUDITORIA_ZIP_ORIGINAL_V6.md) confirmou que **as licenças não acompanham o ZIP atual** e que ele contém um estado de jogo cujos direitos de redistribuição precisam de conferência. Consulte a [revisão de licenças e pendências](docs/REVISAO_LICENCAS.md) antes de qualquer publicação de binários.

Nenhum desses documentos altera o ZIP original. A versão V6 testada deve permanecer preservada, sem substituição por uma reconstrução V5.

## Estado dos testes

O launcher, a configuração de ROM/BIOS e o carregamento do estado compatível foram testados localmente. A compatibilidade entre jogadores no Netplay ainda precisa ser validada com a mesma versão dos componentes, CHD e BIOS.

Este é um projeto comunitário não oficial, sem afiliação declarada com os titulares da franquia.
