# City

A top-down grid-based multiplayer roleplay game inspired by Space Station 13's round structure. Set in a town environment, supporting up to 100 concurrent players in ~3 hour rounds.

## Features

- **Multiplayer**: Authoritative server with client-side prediction for responsive gameplay
- **Grid-based**: Tile-based world with chunk loading
- **Customizable**: Servers fork the codebase to create custom content/experiences
- **Cross-platform**: Windows and Linux support

## Architecture

```
city/
├── src/
│   ├── core/          # Shared engine (ECS, grid, networking, serialization)
│   ├── client/        # Game client (SDL3 + Vulkan rendering)
│   └── server/        # Dedicated server
├── content/           # Base game content (sprites, maps, definitions)
├── tests/             # Unit tests
└── third_party/       # Dependencies (SDL3, ENet, etc.)
```

## Requirements

### Linux (Ubuntu/Debian)
```bash
# Build tools
sudo apt install clang lld lldb cmake ninja-build

# Vulkan development packages
sudo apt install libvulkan-dev vulkan-tools vulkan-validationlayers spirv-tools
```

### Linux (Arch)
```bash
sudo pacman -S clang lld lldb cmake ninja vulkan-devel
```

### Windows
- [LLVM](https://releases.llvm.org/) (includes clang, lld, lldb)
- [CMake](https://cmake.org/)
- [Ninja](https://ninja-build.org/)
- [Vulkan SDK](https://vulkan.lunarg.com/)

### VSCode Extensions
- CMake Tools (`ms-vscode.cmake-tools`)
- CodeLLDB (`vadimcn.vscode-lldb`)
- clangd (`llvm-vs-code-extensions.vscode-clangd`)

## Build

```bash
# Configure (downloads dependencies via FetchContent)
cmake --preset debug          # Client + Server + Tests
cmake --preset client-debug   # Client only
cmake --preset server-debug   # Server only

# Build
cmake --build --preset debug

# Run
./build/debug/city_client     # Client
./build/debug/city_server     # Server
```

## Build Presets

| Preset | Description |
|--------|-------------|
| `debug` | Client + Server + Tests (Debug) |
| `release` | Client + Server (Release) |
| `client-debug` | Client only (Debug) |
| `client-release` | Client only (Release) |
| `server-debug` | Server only (Debug) |
| `server-release` | Server only (Release) |

## Test

```bash
ctest --preset debug
```

## Project Structure

### Core (`src/core/`)
Shared code between client and server:
- **ECS**: Entity Component System with sparse-set storage
- **Grid**: Chunk-based tile map system
- **Net**: Binary serialization, protocol definitions, message types
- **Content**: Asset manifest and loading

### Client (`src/client/`)
- **Render**: SDL3 window + Vulkan rendering
- **Input**: Action-based input handling with keybindings
- **Prediction**: Client-side prediction and server reconciliation
- **Net**: ENet connection to server
- **UI**: Chat window, HUD

### Server (`src/server/`)
- **Simulation**: Authoritative game state, round management
- **Net**: ENet host, client sessions, content serving
- **Systems**: Input processing, entity synchronization

## Technical Details

- **Language**: C++20
- **Tick Rate**: 60 ticks/second
- **Networking**: ENet (reliable UDP)
- **Rendering**: SDL3 + Vulkan
- **Serialization**: Custom binary format

## Server Customization

Servers fork the repository and create custom content:

1. Fork this repository
2. Create `server_content/` with custom assets
3. Build with `-DSERVER_CONTENT_DIR=server_content`
4. Clients download server content on connect

## Debug

In VSCode, press `F5` to build and debug with LLDB.
