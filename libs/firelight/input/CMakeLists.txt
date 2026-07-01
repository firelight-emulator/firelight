cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Sql)
find_package(SDL2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(firelight_input STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/input_mapping.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/shortcut_mapping.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/shortcut_registry.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/shortcut_engine.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/gamepad_profile.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/input_service.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_controller_repository.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/keyboard_input_handler.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/sdl_controller.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/sdl_input_service.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/firelight/input/keyboard_input_handler.hpp
)
target_include_directories(firelight_input
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(firelight_input
        PUBLIC Qt6::Core
        PRIVATE Qt6::Gui
        PRIVATE Qt6::Sql
        PRIVATE SDL2::SDL2
        PRIVATE spdlog::spdlog
        PUBLIC nlohmann_json::nlohmann_json
        PUBLIC firelight_platforms
)

# --- Unit tests (self-contained: link only firelight_input) ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_input_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/test_gamepad.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/input_model_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/input_service_impl_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/controller_repository_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/shortcut_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/gamepad_profile_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/shortcut_registry_test.cpp
)
target_include_directories(firelight_input_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
)
target_link_libraries(firelight_input_test
        PRIVATE firelight_input
        PRIVATE Qt6::Core
        PRIVATE Qt6::Gui
        PRIVATE Qt6::Sql
        PRIVATE SDL2::SDL2
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_input_test)
