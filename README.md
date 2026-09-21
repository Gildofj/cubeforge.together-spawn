# 🌐 CubeForge TogetherSpawn

<div align="center">

![CubeForge](https://img.shields.io/badge/CubeForge-Mod-blue.svg)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%20x64-lightgrey.svg)

**Mod especializado para Cube World focado em corrigir o sistema de spawn em sessões Steam Multiplayer e otimizar conexões P2P.**

</div>

---

## 🎯 Sobre o Mod

No Cube World original (versão Steam), quando novos jogadores entram em uma sessão online através do Steam Friends / P2P, seus personagens frequentemente realizam o primeiro spawn em coordenadas distantes (ou em biomas de início de singleplayer a dezenas de regiões de distância), impossibilitando que amigos joguem juntos sem caminhar por horas.

O **TogetherSpawn** resolve esse problema de forma inteligente e definitiva:
1. **Spawn Inicial Inteligente Próximo ao Host**: Ao entrar pela primeira vez na sessão de um Host, o mod calcula uma posição segura de terreno em raio radial (3 a 5 blocos) ao lado do Host e realiza o teleporte seguro do jogador.
2. **Histórico Persistente de Spawn**: Salva o histórico de sessões (`HostSteamID + WorldSeed + CharacterSlot`) em `together_spawn_config.json`. Reconexões futuras não forçam o teleporte novamente, preservando a posição salva do personagem.
3. **Cálculo de Terreno Seguro (Anti-Sufocamento / Anti-Queda)**: Utiliza verificação de elevação vertical de blocos para garantir que o jogador pouse com os pés no chão sólido e com altura desobstruída.
4. **Proteção Temporária de Aterrissagem**: Concede 5 segundos de invulnerabilidade contra dano de queda e ataque imediato pós-spawn.
5. **Otimização de Conexão Steam P2P**: Auto-aceitação de requisições de sessão P2P (`OnP2PRequest`) para evitar quedas de conexão, desync e telas infinitas de carregamento.
6. **Desbloqueio de Regiões**: Aplica patch no motor do jogo para permitir viagem e spawn livre entre regiões no multiplayer.

---

## 💬 Comandos In-Game

Digite no chat do jogo (pressione `Enter`):

| Comando | Descrição |
| :--- | :--- |
| `/together status` | Exibe o status da sessão (Host/Client), Seed, Host Steam ID e lista de jogadores com distâncias relativas. |
| `/together sync` | Força a sincronização e teleporte para próximo do Host. |
| `/together reset` | Reseta a marcação de "já spawnado" para a sessão atual. |
| `/together radius <N>` | Ajusta o raio de spawn (ex: `/together radius 5`). |
| `/tphost` ou `/spawn` | Teleporta voluntariamente para a posição do Host (sujeito a cooldown configurável). |
| `/tp <nome_do_jogador>` | Teleporta para um amigo conectado na mesma sessão. |
| `/setteamspawn` | *(Host)* Define a posição atual do Host como ponto oficial de spawn do time. |
| `[F6]` | Abre/fecha o menu de configurações rápidas. |

---

## ⚙️ Configurações (`together_spawn_config.json`)

O arquivo de configuração é gerado automaticamente em `%APPDATA%/CubeForge/TogetherSpawn/together_spawn_config.json`:

```json
{
  "auto_spawn_near_host": true,
  "spawn_radius_blocks": 4.0,
  "auto_accept_p2p": true,
  "invulnerability_seconds": 5.0,
  "allow_teleport_commands": true,
  "teleport_cooldown_seconds": 10,
  "spawned_sessions": [
    "76561198000000000_12345678_0"
  ]
}
```

---

## 🛠️ Compilação e Build

### Pré-requisitos
- Visual Studio 2022 (MSVC v143 x64) com suporte a C++20
- CMake 3.25 ou superior
- Ninja Build (recomendado)

### Compilando via PowerShell
```powershell
# Build otimizado para Release
.\build.ps1 -BuildType Release

# Build e instalação direta no Cube World
.\build.ps1 -BuildType Release -InstallPath "C:\Program Files (x86)\Steam\steamapps\common\Cube World"
```

A DLL gerada estará localizada em `dist/Mods/TogetherSpawn.dll`.

---

## 📦 Instalação Manual

1. Instale o **CubeForge Loader** (ou **CubeModLoader**) na pasta raiz do seu Cube World.
2. Copie `TogetherSpawn.dll` para a pasta `Mods/` dentro da pasta do jogo:
   ```
   Cube World/
   ├── CubeWorld.exe
   ├── CubeForgeLoader.dll
   └── Mods/
       └── TogetherSpawn.dll
   ```
3. Inicie o jogo pelo executável do jogo. O mod será carregado automaticamente.

---

## 📜 Licença

Distribuído sob a licença MIT. Consulte `LICENSE` para mais detalhes.
