# city

A cross-platform C++20 project using LLVM/Clang toolchain.

## Requirements

### Linux (Ubuntu/Debian)
```bash
sudo apt install clang lld lldb cmake ninja-build
```

### Linux (Arch)
```bash
sudo pacman -S clang lld lldb cmake ninja
```

### Windows
- [LLVM](https://releases.llvm.org/) (includes clang, lld, lldb)
- [CMake](https://cmake.org/)
- [Ninja](https://ninja-build.org/)

### VSCode Extensions
- CMake Tools (`ms-vscode.cmake-tools`)
- CodeLLDB (`vadimcn.vscode-lldb`)
- clangd (`llvm-vs-code-extensions.vscode-clangd`)

## Build

```bash
# Configure
cmake --preset debug    # or: cmake --preset release

# Build
cmake --build --preset debug

# Run
./build/debug/city      # Linux
.\build\debug\city.exe  # Windows
```

## Test

```bash
ctest --preset debug
```

## Debug

In VSCode, press `F5` to build and debug with LLDB.
