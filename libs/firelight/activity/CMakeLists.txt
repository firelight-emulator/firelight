cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 20)

add_library(firelight_activity STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_activity_log.cpp
)

target_include_directories(firelight_activity
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(firelight_activity
        PRIVATE spdlog::spdlog
        PRIVATE Qt6::Sql
)