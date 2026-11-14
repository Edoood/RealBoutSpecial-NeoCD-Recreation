# Estado da publicação

O Real Bout Special NeoCD Recreation é um projeto comunitário de emulação para Windows. O código do launcher, os patches recuperados e as instruções de compilação estão no repositório. Cada usuário fornece seu CHD e suas BIOS. Consulte as [licenças de terceiros](../THIRD_PARTY_LICENSES/) e as [instruções de reconstrução](../recovered-v5/README.md).

## Reconstrução experimental

O artefato da [execução 43291616227](https://github.com/Edoood/RealBoutSpecial-NeoCD-Recreation/actions/runs/43291616227) foi verificado pelo SHA-256 do GitHub e testado no Windows em uma pasta isolada. O teste confirmou a alternância 320 → 426 → 320 com RAM idêntica, a restauração de estado com reprodução de 60 quadros e a geração de amostras de áudio não nulas. As capturas da tela de título têm centro idêntico e faixas laterais pretas em 426×224. A avaliação durante lutas e a comparação funcional com a V6 seguem pendentes.

Para executar os testes localmente, use uma pasta fora do repositório. O núcleo precisa de `neocdz.zip` e `neogeo.zip` em `rffs-selftest/system/fbneo` e do CHD na raiz da pasta de teste.

## Release binária V6

O pacote v1.0.0 está em rascunho. Para a distribuição binária, seguem pendentes a identificação dos fontes correspondentes à V6, a verificação das condições Steamworks e do AppID 480 e a revisão dos direitos de redistribuição do savestate e demais componentes. A cópia auditada com os avisos de licença está descrita na [revisão de licenças](REVISAO_LICENCAS.md). A [proveniência](PROVENIENCIA_CODIGO_V6.md) registra os hashes e a origem dos fontes disponíveis.
