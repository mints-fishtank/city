# Client executable

add_executable(city_client
    main.cpp
    client.cpp

    # Rendering (using SDL2D for now, Vulkan later)
    render/renderer.cpp

    # Input
    input/input_manager.cpp

    # Prediction
    prediction/input_buffer.cpp
    prediction/prediction.cpp
    prediction/interpolation.cpp

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
    city_server_lib
    SDL3::SDL3
    enet
    stb
)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_link_libraries(city_client PRIVATE pthread)
elseif(WIN32)
    target_link_libraries(city_client PRIVATE winmm)
endif()
