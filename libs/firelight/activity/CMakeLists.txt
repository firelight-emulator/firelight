cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 20)

add_library(firelight_activity STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_activity_log.cpp
)

target_include_directories(firelight_achievements
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(firelight_achievements
        PRIVATE spdlog::spdlog
        PRIVATE Qt6::Gui
        PRIVATE Qt6::Sql
)