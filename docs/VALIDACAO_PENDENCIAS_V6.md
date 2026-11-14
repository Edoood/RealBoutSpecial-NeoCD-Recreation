# Validação das pendências da V6 — 14/11/2026

| Item | Resultado verificado | Evidência que falta |
| --- | --- | --- |
| CHD e BIOS | O inventário anterior, vinculado ao SHA-256 do ZIP original, não lista CHD ou BIOS. | Para qualquer novo ZIP, repetir o inventário sobre o arquivo final. |
| Avisos de licença | O rascunho lista dois anexos. O original não traz os avisos. A auditoria anterior registrou FBNeo, GGPO, SDL3 e ImGui na cópia revisada. | Conferir os avisos adicionais de cada componente efetivamente distribuído. |
| Fonte do núcleo | O patch local recuperado e o patch no GitHub têm o mesmo identificador Git `c90934f7e6c5d14fb16dc3ec2d53e239e1131043`. O patch define e usa `RffsWideSprites[384]`. O arquivo de desenvolvimento `teste.7z` contém fontes do player e um patch widescreen. | Identificar o snapshot e os insumos que produziram a DLL e o EXE da V6. O nome exportado e a identidade do patch com a cópia do GitHub não vinculam o patch ao binário da V6. |
| Savestate | O inventário registra 8.974.473 bytes. A busca local encontrou dois SHA-256 distintos entre estados com esse tamanho. O player recuperado grava um estado serializado do núcleo, em formato próprio com checagem de compatibilidade. | Identificar a amostra exata pelo hash do membro do ZIP, mapear as regiões serializadas e confirmar a base de direitos para distribuir o conteúdo. |
| Arte e controles | O inventário registra `Support/banner.bmp` e `Support/rffs-runtime/gamecontrollerdb.txt`. O projeto upstream do banco conhecido publica licença própria. | Identificar a origem e a autorização do banner, e vincular o banco entregue a uma revisão upstream com seu aviso de licença. |
| Visual C++ | A Microsoft prevê redistribuição sujeita aos termos da licença Visual Studio e à lista REDIST da versão aplicável. | Identificar versão, origem e condições aplicáveis às três DLLs do ZIP. |
| Steamworks | A integração do projeto principal permanece ativa. O AppID 480 pertence ao exemplo Spacewar. O mantenedor informou que não tem autorização da Valve. | Autorização para o uso e distribuição pretendidos continua sem comprovação. |

A página Releases mostrou ambos os anexos em rascunho. As tentativas de baixar os anexos pelo navegador não produziram arquivos locais acessíveis nesta sessão. As conclusões sobre seu inventário usam a auditoria anterior vinculada ao hash do ZIP original. A inspeção independente do conteúdo binário exato do savestate e da arte segue pendente. Nenhum anexo, binário, savestate ou configuração Steam foi editado.

[Auditoria do ZIP](AUDITORIA_ZIP_ORIGINAL_V6.md) · [Proveniência](PROVENIENCIA_CODIGO_V6.md) · [Licença FBNeo](../THIRD_PARTY_LICENSES/FBNeo-license.txt) · [Microsoft REDIST](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files) · [Banco de controles upstream](https://github.com/mdqinc/SDL_GameControllerDB/blob/master/LICENSE) · [Valve Spacewar](https://partner.steamgames.com/doc/sdk/api/example)
