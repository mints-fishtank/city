# Client executable

add_executable(city_client
    main.cpp
    client.cpp

    # Rendering
    render/renderer.cpp
    render/sprite_batch.cpp
    render/tile_renderer.cpp
    render/vulkan/vk_context.cpp

    # Input
    input/input_manager.cpp

    # Prediction
    prediction/input_buffer.cpp
    prediction/prediction.cpp

    # Networking
    net/client_connection.cpp
    net/content_downloader.cpp

    # UI
    ui/chat_window.cpp
)

target_include_directories(city_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(city_client PRIVATE
    city_core
    SDL3::SDL3
    enet
    stb
)

# Find Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(city_client PRIVATE Vulkan::Vulkan)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_link_libraries(city_client PRIVATE pthread)
endif()
