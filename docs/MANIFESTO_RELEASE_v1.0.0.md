# Manifesto da versão funcional v1.0.0

Este manifesto identifica os arquivos do ZIP de teste **RealBoutSpecial_NeoCD_Recreation_Pacote_Funcional.zip**. Os SHA-256 documentam integridade, não licença ou autorização.

| Caminho | SHA-256 |
| --- | --- |
| RealBoutLauncher.exe | 11550860d281c877ed73e0689e87ce9c04313a497ac96de7526d4b5dbefc822e |
| Support/Jogar Real Bout Online.exe | 671768d621fe4ea13f55a9face89da4725343ea10600df7a6625fa73ada82cf6 |
| Support/rffs-runtime/rffs_wide_v5.dll | 61a02a4c4ee0150423b5a15180b1652c88793a006a3c8465fc819dce49bc7434 |
| Support/SDL3.dll | 2b4b128d8d510cc2d442693cf848c050d700d43dc667afe9e5fd82b0570d667e |
| Support/steam_api64.dll | e6d9bafb9a41e42fba7b21553db49f8719027af3f0deb23ff86cfc44d60e776d |
| Support/msvcp140.dll | 7c26614e1d733892c2deac7e245ce115504b1d80592dd0a01b08e3e5a55f89ca |
| Support/vcruntime140.dll | d1f4225df2cd877dbf130d5668a021dce3f94118455ff5ec952061c30afc9ce7 |
| Support/vcruntime140_1.dll | a7146c08f89fe5b04541ab507cdb59ff7b44534d4ba3c668a426c6450a03434e |
| Support/rffs-runtime/estado-inicial-protegido.rffsstate | 76247262bee61ecc3c7c9745ec65ed77ad48cbeb0e4256afeb11c5608141a3c8 |
| Support/rffs-runtime/system/fbneo/ngcd_rbffspec.fs | 4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe |

## Situação de publicação

- ROM CHD e arquivos BIOS: ausentes do pacote.
- Código-fonte do launcher: presente em `src/RealBoutLauncher.cpp`.
- Código-fonte completo correspondente ao executável do jogo e à DLL: **não encontrado nos arquivos recebidos**.
- Licença FBNeo: [cópia no repositório](../THIRD_PARTY_LICENSES/FBNeo-license.txt); **ainda é preciso incluí-la no próprio ZIP distribuído**.
- Save state e arquivo `.fs`: conteúdo/redistribuição ainda não auditados.
- Steamworks SDK e redistribuição do `steam_api64.dll`: requer conferência dos termos aplicáveis.

**Manifesto não certifica liberação pública**. A revisão de licenças completa está em [REVISAO_LICENCAS.md](REVISAO_LICENCAS.md).
