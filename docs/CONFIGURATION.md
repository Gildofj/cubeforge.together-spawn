# ⚙️ Guia de Configuração - TogetherSpawn

Este guia fornece uma referência completa sobre o arquivo de configuração `together_spawn_config.json`, suas chaves, tipos de dados, valores padrão e comportamento em tempo de execução.

---

## 📑 Sumário
- [1. Localização do Arquivo](#1-localização-do-arquivo)
- [2. Estrutura do Arquivo JSON](#2-estrutura-do-arquivo-json)
- [3. Descrição Detalhada dos Parâmetros](#3-descrição-detalhada-dos-parâmetros)
  - [`auto_spawn_near_host`](#auto_spawn_near_host)
  - [`spawn_radius_blocks`](#spawn_radius_blocks)
  - [`auto_accept_p2p`](#auto_accept_p2p)
  - [`invulnerability_seconds`](#invulnerability_seconds)
  - [`allow_teleport_commands`](#allow_teleport_commands)
  - [`teleport_cooldown_seconds`](#teleport_cooldown_seconds)
  - [`spawned_sessions`](#spawned_sessions)
- [4. Gerenciamento do Histórico de Sessões](#4-gerenciamento-do-histórico-de-sessões)
- [5. Edição Segura e Recarregamento](#5-edição-segura-e-recarregamento)

---

## 1. Localização do Arquivo

O arquivo de configuração é criado automaticamente no primeiro início do mod no seguinte caminho padrão do Windows:

```
%APPDATA%\CubeForge\TogetherSpawn\together_spawn_config.json
```

Caminho absoluto típico:
```
C:\Users\<SeuUsuario>\AppData\Roaming\CubeForge\TogetherSpawn\together_spawn_config.json
```

> [!TIP]
> Você pode abrir essa pasta rapidamente pressionando **`Win + R`**, digitando `%APPDATA%\CubeForge\TogetherSpawn` e pressionando **`Enter`**.

---

## 2. Estrutura do Arquivo JSON

Abaixo está um exemplo de configuração padrão gerada pelo mod:

```json
{
  "auto_spawn_near_host": true,
  "spawn_radius_blocks": 4.0,
  "auto_accept_p2p": true,
  "invulnerability_seconds": 5.0,
  "allow_teleport_commands": true,
  "teleport_cooldown_seconds": 10,
  "spawned_sessions": [
    "76561198012345678_26879_0"
  ]
}
```

---

## 3. Descrição Detalhada dos Parâmetros

### `auto_spawn_near_host`
- **Tipo**: `boolean` (`true` ou `false`)
- **Padrão**: `true`
- **Descrição**: Habilita o sistema automático de detecção e teleporte de primeiro spawn ao entrar na sessão de um Host.
- **Efeito ao desativar (`false`)**: O mod não fará o teleporte automático na primeira entrada; o jogador permanecerá no ponto nativo gerado pelo jogo, a menos que utilize comandos manuais como `/together sync` ou `/tphost`.

---

### `spawn_radius_blocks`
- **Tipo**: `float` (número decimal ou inteiro)
- **Padrão**: `4.0`
- **Valores recomendados**: Entre `2.0` e `10.0` (o comando in-game aceita de `1.0` a `50.0`).
- **Descrição**: O raio radial horizontal (em blocos de Cube World) ao redor do Host ou alvo onde o mod buscará um piso sólido e seguro para posicionar o jogador.
- **Vantagem**: Evita que múltiplos jogadores surjam exatamente na mesma coordenada, prevenindo colisões físicas anômalas no motor do jogo.

---

### `auto_accept_p2p`
- **Tipo**: `boolean` (`true` ou `false`)
- **Padrão**: `true`
- **Descrição**: Intercepta o callback do Steamworks `OnP2PRequest` e aceita automaticamente qualquer solicitação de sessão P2P de jogadores conectados.
- **Vantagem**: Elimina desincronizações (desync), desconexões prematuras e telas de carregamento infinitas comuns no multiplayer original do Cube World.

---

### `invulnerability_seconds`
- **Tipo**: `float` (segundos)
- **Padrão**: `5.0`
- **Descrição**: Duração do bônus de armadura e resistência ($+999.999$) concedido ao jogador após qualquer spawn ou teleporte realizado pelo mod.
- **Vantagem**: Garante tempo suficiente para carregar blocos locais do mundo sem risco de morrer por queda durante o carregamento de chunk ou por ataques de criaturas inimigas próximas.

---

### `allow_teleport_commands`
- **Tipo**: `boolean` (`true` ou `false`)
- **Padrão**: `true`
- **Descrição**: Permite o uso dos comandos de teleporte voluntário `/tphost`, `/spawn` e `/tp <nome>`.
- **Efeito ao desativar (`false`)**: Apenas o spawn inicial automático funcionará; comandos manuais de teleporte serão desativados caso você queira uma experiência de viagem mais tradicional.

---

### `teleport_cooldown_seconds`
- **Tipo**: `integer` (segundos)
- **Padrão**: `10`
- **Descrição**: Tempo de espera obrigatório entre duas execuções consecutivas de comandos de teleporte por um mesmo jogador.

---

### `spawned_sessions`
- **Tipo**: `array` de `strings`
- **Padrão**: `[]` (vazio inicialmente)
- **Descrição**: Lista de chaves de sessões que já tiveram seu primeiro spawn concluído com sucesso.
- **Estrutura de cada chave**:
  ```text
  "<HostSteamID>_<WorldSeed>_<CharacterSlot>"
  ```
  - `HostSteamID`: O ID de 64 bits da conta Steam do anfitrião do jogo.
  - `WorldSeed`: O número de semente aleatória gerado para aquele mundo.
  - `CharacterSlot`: O índice do personagem utilizado (0 a 7).

---

## 4. Gerenciamento do Histórico de Sessões

O TogetherSpawn mantém o histórico no array `spawned_sessions` para diferenciar a **primeira entrada** de uma **reconexão**:

1. **Primeira Entrada**: O mod detecta que a combinação `Host + Seed + Slot` não está na lista. O teleporte seguro é acionado e a chave é gravada no arquivo.
2. **Reconexões Futuras**: O mod detecta que a chave já existe. Ele **não** move o jogador, permitindo que você continue de onde parou na sessão anterior.
3. **Reset Manual**:
   - Para redefinir uma sessão específica no jogo, use `/together reset`.
   - Para limpar todo o histórico de todos os mundos, você pode editar o arquivo e deixar `"spawned_sessions": []`.

---

## 5. Edição Segura e Recarregamento

- Você pode editar o arquivo `together_spawn_config.json` com qualquer editor de texto (VS Code, Notepad++, Bloco de Notas).
- **Em Execução**: Alterações feitas através de comandos in-game (ex: `/together radius 8`) gravam o arquivo imediatamente de forma *thread-safe*.
- **Edição Manual Externa**: Recomendamos editar com o jogo fechado ou antes de entrar em uma sessão multiplayer para garantir que as alterações sejam carregadas na inicialização.
