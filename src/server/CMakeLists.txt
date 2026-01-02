# Server library (shared between standalone server and client's embedded server)

add_library(city_server_lib STATIC
    server.cpp

    # Simulation
    simulation/game_state.cpp
    simulation/round_manager.cpp

    # Networking
    net/server_connection.cpp
    net/client_session.cpp
    net/content_server.cpp

    # Systems
    systems/input_processor.cpp
    systems/entity_sync.cpp
)

target_include_directories(city_server_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(city_server_lib PUBLIC
    city_core
    enet
)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_link_libraries(city_server_lib PUBLIC pthread)
endif()

# Server executable
add_executable(city_server main.cpp)

target_link_libraries(city_server PRIVATE city_server_lib)
