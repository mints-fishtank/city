cmake_minimum_required(VERSION 3.21)
project(city LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_CLIENT "Build game client" ON)
option(BUILD_SERVER "Build dedicated server" ON)
option(BUILD_TESTING "Build tests" ON)
option(BUILD_TOOLS "Build asset tools" OFF)

# Custom content directory (for server forks)
set(CONTENT_DIR "${CMAKE_SOURCE_DIR}/content" CACHE PATH "Content directory")
set(SERVER_CONTENT_DIR "" CACHE PATH "Server-specific content overlay")

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

# Third-party dependencies
add_subdirectory(third_party)

# Core library (shared between client and server)
add_subdirectory(src/core)

# Client executable
if(BUILD_CLIENT)
    add_subdirectory(src/client)
endif()

# Server executable
if(BUILD_SERVER)
    add_subdirectory(src/server)
endif()

# Tools
if(BUILD_TOOLS)
    add_subdirectory(tools)
endif()

# Testing
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
