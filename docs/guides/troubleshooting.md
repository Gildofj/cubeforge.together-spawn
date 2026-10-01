# 🔧 Guia de Resolução de Problemas (Troubleshooting)

Este guia reúne soluções para os problemas e dúvidas mais comuns encontrados durante o uso ou desenvolvimento do mod **CubeForge TogetherSpawn**.

---

## 📑 Sumário
- [1. Problemas de Instalação e Carregamento](#1-problemas-de-instalação-e-carregamento)
  - [O mod não aparece como carregado no jogo](#o-mod-não-aparece-como-carregado-no-jogo)
  - [Erro de falta de DLLs do Visual C++](#erro-de-falta-de-dlls-do-visual-c)
- [2. Problemas de Spawn e Teleporte](#2-problemas-de-spawn-e-teleporte)
  - [Entre no servidor e não fui teleportado automaticamente](#entre-no-servidor-e-não-fui-teleportado-automaticamente)
  - [O jogador apareceu dentro de uma montanha ou caindo no vazio](#o-jogador-apareceu-dentro-de-uma-montanha-ou-caindo-no-vazio)
  - [Mensagem "Comando em cooldown"](#mensagem-comando-em-cooldown)
- [3. Problemas de Conexão e P2P Multiplayer](#3-problemas-de-conexão-e-p2p-multiplayer)
  - [Mensagem "Host não encontrado" ou "Host world is not ready"](#mensagem-host-não-encontrado-ou-host-world-is-not-ready)
  - [Desconexões frequentes em sessões Steam](#desconexões-frequentes-em-sessões-steam)
- [4. Problemas de Compilação e Build](#4-problemas-de-compilação-e-build)
  - [Erro "vcvars64.bat não encontrado"](#erro-vcvars64bat-não-encontrado)
  - [Erro na resolução da CWSDK via CMake](#erro-na-resolução-da-cwsdk-via-cmake)

---

## 1. Problemas de Instalação e Carregamento

### O mod não aparece como carregado no jogo
- **Causa**: O Mod Loader não está presente ou a DLL está na pasta errada.
- **Solução**:
  1. Certifique-se de que o **CubeForge Loader** (`CubeForgeLoader.dll` ou `CubeModLoader.fip`) está instalado na pasta raiz do Cube World (`C:\...\Cube World\`).
  2. Verifique se o arquivo `TogetherSpawn.dll` está dentro da subpasta `Mods/`:
     ```
     Cube World/
     ├── CubeWorld.exe
     └── Mods/
         └── TogetherSpawn.dll
     ```
  3. No jogo, abra o chat com `Enter` e digite `/together status`. Se o mod estiver ativo, ele responderá com o cabeçalho verde/ciano.

### Erro de falta de DLLs do Visual C++
- **Causa**: Ausência do pacote de redistribuíveis Microsoft Visual C++ x64.
- **Solução**: Baixe e instale a versão mais recente do [Visual C++ Redistributable (x64)](https://aka.ms/vs/17/release/vc_redist.x64.exe).

---

## 2. Problemas de Spawn e Teleporte

### Entrei no servidor e não fui teleportado automaticamente
- **Causa 1**: Você já jogou nesta mesma sessão (mesmo Host e Seed) com este personagem anteriormente, então o mod preservou sua posição salva.
  - **Solução**: Digite `/together reset` no chat e depois `/together sync`, ou use diretamente `/tphost`.
- **Causa 2**: O Host demorou para carregar o mapa.
  - **Solução**: Digite `/together sync` assim que o mundo carregar.

### O jogador apareceu dentro de uma montanha ou caindo no vazio
- **Causa**: O chunk onde o Host estava ainda não havia sido gerado completamente pelo motor de terreno no momento do teleporte.
- **Solução**:
  1. A proteção de aterrissagem concede 5 segundos de invulnerabilidade com $+999.999$ de armadura para evitar morte por queda.
  2. Digite `/together sync` ou `/tphost` para recalcular a posição em piso sólido.

### Mensagem "Comando em cooldown"
- **Causa**: Há um intervalo de espera de segurança entre comandos consecutivos de teleporte.
- **Solução**: Aguarde os segundos indicados na mensagem ou reduza o valor de `"teleport_cooldown_seconds"` em `together_spawn_config.json`.

---

## 3. Problemas de Conexão e P2P Multiplayer

### Mensagem "Host não encontrado" ou "Host world is not ready"
- **Causa**: O cliente enviou um pacote P2P antes do Host carregar a entidade principal do jogador no mapa.
- **Solução**: Aguarde o Host concluir a tela inicial de carregamento e execute `/together sync`.

### Desconexões frequentes em sessões Steam
- **Causa**: Bloqueio de portas UDP ou falha no handshake P2P.
- **Solução**:
  1. Certifique-se de que a opção `"auto_accept_p2p": true` está habilitada no arquivo de configuração tanto no Host quanto nos Clientes.
  2. Adicione o executável `CubeWorld.exe` nas exceções do Firewall do Windows.

---

## 4. Problemas de Compilação e Build

### Erro "vcvars64.bat não encontrado"
- **Causa**: O ambiente de linha de comando do Visual Studio não foi detectado automaticamente pelo PowerShell.
- **Solução**: Abra o **x64 Native Tools Command Prompt for VS 2022** pelo menu Iniciar do Windows e execute `powershell .\build.ps1`.

### Erro na resolução da CWSDK via CMake
- **Causa**: O CMake não conseguiu clonar a SDK nem localizá-la nos diretórios locais.
- **Solução**:
  - Certifique-se de que a pasta `cubeforge.sdk` está clonada ao lado da pasta do mod (`d:\Projects\cubeforge.sdk`), ou passe explicitamente o caminho:
    ```powershell
    cmake -B build -DCUBEFORGE_SDK_PATH="D:/Projects/cubeforge.sdk"
    ```
