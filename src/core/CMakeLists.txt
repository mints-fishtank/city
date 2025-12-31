# Core library - shared between client and server

add_library(city_core STATIC
    # Utilities
    util/types.cpp

    # Serialization
    net/serialization.cpp
    net/message.cpp

    # ECS
    ecs/world.cpp

    # Grid
    grid/chunk.cpp
    grid/tilemap.cpp

    # Content
    content/content_manifest.cpp
    content/content_loader.cpp

    # Game systems
    game/systems/movement.cpp
)

target_include_directories(city_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(city_core PUBLIC
    nlohmann_json::nlohmann_json
    libzstd_static
)

# Ensure C++20
target_compile_features(city_core PUBLIC cxx_std_20)
