# 🤝 Guia de Contribuição - CubeForge TogetherSpawn

Agradecemos pelo interesse em contribuir com o **CubeForge TogetherSpawn**! Este documento orienta desenvolvedores sobre a configuração do ambiente, o fluxo de build, padrões de código e o processo de submissão de melhorias.

---

## 📑 Sumário
- [1. Pré-requisitos de Desenvolvimento](#1-pré-requisitos-de-desenvolvimento)
- [2. Configuração do Ambiente](#2-configuração-do-ambiente)
- [3. Fluxo de Compilação e Build](#3-fluxo-de-compilação-e-build)
  - [Utilizando o Script PowerShell (`build.ps1`)](#utilizando-o-script-powershell-buildps1)
  - [Utilizando CMake Presets Diretamente](#utilizando-cmake-presets-diretamente)
- [4. Estrutura do Projeto e Localização da SDK](#4-estrutura-do-projeto-e-localização-da-sdk)
- [5. Padrões de Código e Boas Práticas](#5-padrões-de-código-e-boas-práticas)
  - [Padrões C++20](#padrões-c20)
  - [Segurança de Memória e Ponteiros](#segurança-de-memória-e-ponteiros)
  - [Protocolo Binário de Rede (P2P)](#protocolo-binário-de-rede-p2p)
  - [Logging e Mensagens ao Usuário](#logging-e-mensagens-ao-usuário)
- [6. Como Testar as Modificações](#6-como-testar-as-modificações)
- [7. Fluxo de Pull Requests (PR)](#7-fluxo-de-pull-requests-pr)

---

## 1. Pré-requisitos de Desenvolvimento

Para compilar e desenvolver o TogetherSpawn, certifique-se de possuir:

- **Sistema Operacional**: Windows 10/11 x64
- **Compilador C++**: Visual Studio 2022 (MSVC v143 x64) com suporte completo a **C++20**
- **CMake**: Versão `3.25` ou superior instalada e acessível no `PATH`
- **Gerador de Build**: [Ninja](https://ninja-build.org/) (instalado nativamente pelo instalador do Visual Studio)
- **Controle de Versão**: Git

---

## 2. Configuração do Ambiente

Clone o repositório com suporte a submódulos ou dependências:

```powershell
git clone https://github.com/Gildofj/cubeforge.together-spawn.git
cd cubeforge.together-spawn
```

---

## 3. Fluxo de Compilação e Build

### Utilizando o Script PowerShell (`build.ps1`)

O projeto possui um script automatizado (`build.ps1`) que detecta automaticamente o ambiente MSVC x64 e executa as etapas de configuração, compilação e cópia dos artefatos.

```powershell
# 1. Compilação padrão (Release x64)
.\build.ps1 -BuildType Release

# 2. Compilação em modo Debug (com símbolos e diagnósticos)
.\build.ps1 -BuildType Debug

# 3. Limpeza completa dos diretórios build/ e dist/
.\build.ps1 -Target clean

# 4. Compilação e instalação direta na pasta do Cube World
.\build.ps1 -BuildType Release -InstallPath "C:\Program Files (x86)\Steam\steamapps\common\Cube World"
```

A DLL gerada estará localizada em:
```
dist/Mods/TogetherSpawn.dll
```

### Utilizando CMake Presets Diretamente

O projeto implementa `CMakePresets.json` compatível com CMake 3.25+:

```powershell
# Configuração
cmake --preset windows-release

# Compilação paralela
cmake --build --preset windows-release --parallel
```

Presets disponíveis:
- `windows-release`: Otimização máxima `/O2`, `/GL`, Link-Time Optimization (LTO).
- `windows-debug`: Símbolos de depuração completos `/Od`, `/Zi`.
- `windows-relwithdebinfo`: Otimizado com geração de arquivo `.pdb`.

---

## 4. Estrutura do Projeto e Localização da SDK

O arquivo `CMakeLists.txt` resolve a dependência da **CubeForge SDK (CWSDK)** automaticamente na seguinte ordem de prioridade:

1. Variável de ambiente/CMake `-DCUBEFORGE_SDK_PATH="<caminho>"`
2. Diretório irmão local `../cubeforge.sdk`
3. Diretório irmão local `../CWSDK`
4. Download automático via `FetchContent` a partir do repositório oficial GitHub (`Gildofj/cubeforge.sdk`).

---

## 5. Padrões de Código e Boas Práticas

### Padrões C++20
- Utilize recursos modernos: `std::string_view`, `std::optional`, `constexpr`, `std::filesystem`, structured binding (`auto [k, v]`).
- Evite ponteiros brutos gerenciados manualmente; utilize RAII e referências.
- Respeite o padrão de classes e métodos em **PascalCase** e variáveis membro em `m_camelCase`.

### Segurança de Memória e Ponteiros
O Cube World é um executável de 64 bits sem proteções de bounds. Sempre valide ponteiros da engine antes do acesso:

```cpp
// ❌ Incorreto (Risco de Crash / Null Pointer Dereference)
LongVector3 pos = game->world->local_creature->entity_data.position;

//  Correto
if (!game || !game->world || !game->world->local_creature) {
    return;
}
LongVector3 pos = game->world->local_creature->entity_data.position;
```

### Protocolo Binário de Rede (P2P)
Ao adicionar novos pacotes em `src/features/network/NetworkProtocol.h`:
1. Use alinhamento de 1 byte com `#pragma pack(push, 1)`.
2. Garanta que o cabeçalho contenha o `magic` (`0x43575453`) e valide o `version`.
3. Novos tipos de pacotes devem ser registrados no enum `PacketType`.

```cpp
#pragma pack(push, 1)
struct MyNewPacket {
    PacketHeader header{PROTOCOL_MAGIC, PROTOCOL_VERSION, PacketType::MyNewType, 0};
    uint64_t steamID{0};
    // ... campos de tamanho fixo
};
#pragma pack(pop)
```

### Logging e Mensagens ao Usuário
- Para logs técnicos e diagnósticos de terminal/arquivo, utilize:
  ```cpp
  TogetherSpawn::Utils::Logger::Info("Mensagem informativa");
  TogetherSpawn::Utils::Logger::Warn("Aviso importante");
  TogetherSpawn::Utils::Logger::Error("Falha na operacao");
  TogetherSpawn::Utils::Logger::Debug("Dado detalhado de debug");
  ```
- Para mensagens in-game no chat do jogador, utilize `Utils::PrintChat` com as cores temáticas de `Utils::Colors`.

---

## 6. Como Testar as Modificações

1. **Compilar a DLL**: Execute `.\build.ps1 -BuildType Release`.
2. **Instalar no Jogo**: Copie `dist/Mods/TogetherSpawn.dll` para a pasta `Mods/` do Cube World com o **CubeForge Loader** instalado.
3. **Validar no Jogo**:
   - Inicie o jogo e abra o chat (`Enter`).
   - Digite `/together status` para validar a inicialização do mod.
   - Abra o menu com **`F6`**.
   - Conecte-se a uma sessão Steam com um amigo para testar a sincronização P2P e o teleporte automático.

---

## 7. Fluxo de Pull Requests (PR)

1. Crie uma branch a partir da `master`:
   ```bash
   git checkout -b feature/minha-nova-funcionalidade
   ```
2. Realize suas alterações respeitando os padrões de formatação e sem introduzir warnings no MSVC `/W4`.
3. Certifique-se de que a compilação com `build.ps1` é bem-sucedida em `Release` e `Debug`.
4. Atualize a documentação em `docs/` caso sua alteração adicione comandos, configurações ou modifique a arquitetura.
5. Abra um Pull Request com descrição clara do problema resolvido e passos de teste realizados.
