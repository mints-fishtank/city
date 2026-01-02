# Third-party dependencies using FetchContent
include(FetchContent)

# ImGui - immediate mode GUI for debug/profiling
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.6
    GIT_SHALLOW TRUE
)

# SDL3
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)

# ENet - reliable UDP networking
FetchContent_Declare(
    enet_src
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG v1.3.18
    GIT_SHALLOW TRUE
)

# nlohmann/json - JSON parsing
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

# stb - single-file libraries (image loading, etc.)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# zstd - compression
FetchContent_Declare(
    zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG v1.5.5
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR build/cmake
)

# GoogleTest - testing framework
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
)

# Make dependencies available
set(SDL_VULKAN ON CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)

set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)

set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SDL3 json zstd)

if(BUILD_TESTING)
    FetchContent_MakeAvailable(googletest)
endif()

# stb is header-only, just populate it
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()

# Create interface library for stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

# ENet - manually create library (no CMakeLists in original repo)
FetchContent_GetProperties(enet_src)
if(NOT enet_src_POPULATED)
    FetchContent_Populate(enet_src)
endif()

add_library(enet STATIC
    ${enet_src_SOURCE_DIR}/callbacks.c
    ${enet_src_SOURCE_DIR}/compress.c
    ${enet_src_SOURCE_DIR}/host.c
    ${enet_src_SOURCE_DIR}/list.c
    ${enet_src_SOURCE_DIR}/packet.c
    ${enet_src_SOURCE_DIR}/peer.c
    ${enet_src_SOURCE_DIR}/protocol.c
    ${enet_src_SOURCE_DIR}/unix.c
    ${enet_src_SOURCE_DIR}/win32.c
)
target_include_directories(enet PUBLIC ${enet_src_SOURCE_DIR}/include)

# Platform-specific libraries for ENet
if(WIN32)
    target_link_libraries(enet PUBLIC ws2_32 winmm)
    target_compile_definitions(enet PRIVATE _WIN32_WINNT=0x0600)
endif()

# ImGui - build with SDL3 renderer backend
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)
