cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(firelight_settings STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_settings_repository.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/settings_service.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/firelight/settings/sqlite_settings_repository.hpp
)

target_include_directories(firelight_settings
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)

target_link_libraries(firelight_settings
        PRIVATE spdlog::spdlog
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
        PRIVATE Qt6::Core
)
