# 💬 Guia Completo de Comandos - TogetherSpawn

Este documento lista todos os comandos de chat disponibilizados pelo mod **CubeForge TogetherSpawn**, suas permissões, parâmetros e exemplos práticos de uso.

---

## 📑 Sumário
- [1. Visão Geral](#1-visão-geral)
- [2. Comandos Gerais de Sessão (`/together`)](#2-comandos-gerais-de-sessão-together)
  - [`/together status`](#together-status)
  - [`/together sync`](#together-sync)
  - [`/together reset`](#together-reset)
  - [`/together radius <N>`](#together-radius-n)
  - [`/together help`](#together-help)
- [3. Comandos de Teleporte](#3-comandos-de-teleporte)
  - [`/tphost` ou `/spawn`](#tphost-ou-spawn)
  - [`/tp <nome_do_jogador>`](#tp-nome_do_jogador)
- [4. Comandos Exclusivos do Host](#4-comandos-exclusivos-do-host)
  - [`/setteamspawn`](#setteamspawn)
- [5. Teclas de Atalho In-Game](#5-teclas-de-atalho-in-game)
- [6. Sistema de Cooldown e Proteção](#6-sistema-de-cooldown-e-proteção)

---

## 1. Visão Geral

Para executar qualquer comando no jogo:
1. Pressione **`Enter`** para abrir o chat do Cube World.
2. Digite o comando desejado iniciando com barra (`/`).
3. Pressione **`Enter`** para enviar.

> [!NOTE]
> Os comandos do TogetherSpawn são interceptados antes de serem enviados à rede comum do jogo, garantindo execução instantânea e resposta com mensagens coloridas exclusivas na sua tela.

---

## 2. Comandos Gerais de Sessão (`/together`)

### `/together status`
Exibe um relatório detalhado sobre a sessão atual do jogo.

- **Permissão**: Qualquer jogador (Singleplayer, Host ou Client).
- **Informações Exibidas**:
  - **Modo**: `Host (Servidor)`, `Client (Conectado)` ou `SinglePlayer`.
  - **Seed do Mundo**: O seed numérico do mapa atual.
  - **Host SteamID**: O identificador Steam único de quem está hospedando o mundo.
  - **Status do Spawn**: Indica se você já realizou o spawn inicial nesta sessão (`Sim` ou `Não`).
  - **Lista de Jogadores**: Lista todos os jogadores carregados na área, identificando você e a distância em blocos até os demais.

**Exemplo de Saída no Chat:**
```text
--- [TogetherSpawn: Status da Sessao] ---
 Modo: Client (Conectado)
 Seed do Mundo: 26879
 Host SteamID: 76561198012345678
 Spawn inicial concluido: Sim
 Jogadores carregados na area: 2
  * Steve (Voce)
  * Alex [Distancia: 14 blocos]
```

---

### `/together sync`
Força uma requisição imediata de sincronização de spawn via P2P com o Host da sessão.

- **Permissão**: Apenas **Client**.
- **Comportamento**: Envia um pacote `SpawnRequest` ao Host. Ao receber as coordenadas seguras calculadas pelo Host, realiza o teleporte e reinicia o timer de proteção.
- **Quando usar**: Útil caso você tenha ficado preso ou tenha entrado antes do mundo do Host carregar totalmente.

---

### `/together reset`
Reseta o histórico de primeiro spawn para a sessão e personagem atuais.

- **Permissão**: Qualquer jogador.
- **Comportamento**: Remove a chave `<HostSteamID>_<WorldSeed>_<CharacterSlot>` do histórico persistente e redefine o estado local para `Não spawnado`.
- **Efeito**: Na próxima verificação periódica ou ao reconectar, o mod executará o teleporte automático inicial novamente.

---

### `/together radius <N>`
Ajusta a distância radial de spawn em blocos ao redor do alvo.

- **Permissão**: Qualquer jogador (persiste localmente nas configurações).
- **Parâmetros**:
  - `<N>`: Número inteiro ou decimal entre `1.0` e `50.0` (Padrão: `4.0` blocos).
- **Exemplo**:
  ```text
  /together radius 6
  ```
- **Resposta**:
  ```text
  [TogetherSpawn] Raio de spawn ajustado para 6 blocos.
  ```

---

### `/together help`
Exibe no chat a lista resumida de todos os comandos suportados e suas descrições.

---

## 3. Comandos de Teleporte

### `/tphost` ou `/spawn`
Teleporta o jogador diretamente para a posição atual do Host do servidor.

- **Permissão**: Apenas **Client**.
- **Comportamento**:
  - Se o Host estiver na mesma região (carregado localmente), calcula a posição de solo seguro imediatamente e teleporta.
  - Se o Host estiver em outra região distante (fora da view distance local), consulta as coordenadas do Host via protocolo P2P no canal `42` e teleporta assim que a resposta for recebida.
- **Regras**: Sujeito ao tempo de recarga (*cooldown*) configurado (padrão: 10 segundos).

---

### `/tp <nome_do_jogador>`
Teleporta você para a localização de um amigo conectado na mesma sessão.

- **Permissão**: Qualquer jogador em multiplayer.
- **Parâmetros**:
  - `<nome_do_jogador>`: Nome do personagem (a busca não diferencia maiúsculas de minúsculas e aceita correspondência parcial).
- **Exemplo**:
  ```text
  /tp Arthas
  ```
- **Comportamento**:
  - Se o jogador estiver na área visível local, teleporta imediatamente para um bloco seguro adjacente a ele.
  - Se o jogador estiver em outra região, o cliente solicita as coordenadas do jogador ao Host via P2P.

---

## 4. Comandos Exclusivos do Host

### `/setteamspawn`
Define a posição geográfica exata atual do Host como o ponto de spawn oficial permanente para todos os jogadores do time nesta sessão.

- **Permissão**: Apenas **Host**.
- **Comportamento**:
  - Grava as coordenadas `(X, Y, Z)` no arquivo de configuração do Host.
  - Qualquer novo jogador que entrar no mundo (ou usar `/together sync`) será alocado exatamente neste ponto fixo, em vez de ao lado da posição dinâmica do Host.
- **Resposta**:
  ```text
  [TogetherSpawn] Ponto de Spawn do Time definido na sua posicao atual!
  ```

---

## 5. Teclas de Atalho In-Game

| Tecla | Função |
| :---: | :--- |
| **`F6`** | Abre / fecha o menu rápido de configurações overlay. |
| **`Enter`** | Abre o chat do jogo para digitação de comandos. |

---

## 6. Sistema de Cooldown e Proteção

### Tempo de Recarga (Cooldown)
Para evitar abusos ou travamentos de colisão no motor do jogo, os comandos `/tphost`, `/spawn` e `/tp` possuem um tempo de recarga configurável (`teleport_cooldown_seconds`, padrão: `10` segundos).
- Caso o comando seja chamado antes do tempo, uma mensagem em laranja indicará o tempo restante:
  ```text
  [TogetherSpawn] Comando em cooldown. Aguarde 6s.
  ```

### Proteção de Aterrissagem (Invulnerabilidade)
Após qualquer spawn inicial ou teleporte realizado pelo mod, o personagem recebe **5 segundos de imunidade temporária**:
- **Armadura Adicional**: $+999.999$
- **Resistência Elemental**: $+999.999$
- **Benefício**: Evita mortes instantâneas por dano de queda ou ataques surpresa de monstros no momento do surgimento.
