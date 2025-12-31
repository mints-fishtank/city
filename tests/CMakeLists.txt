# Unit tests

# GoogleTest is included from third_party
include(GoogleTest)

add_executable(city_tests
    core/test_serialization.cpp
    core/test_ecs.cpp
    core/test_grid.cpp
)

target_link_libraries(city_tests PRIVATE
    city_core
    GTest::gtest
    GTest::gtest_main
)

target_include_directories(city_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(city_tests)
