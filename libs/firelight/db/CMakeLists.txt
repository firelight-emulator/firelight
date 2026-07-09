cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(spdlog REQUIRED)

add_library(firelight_db STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_userdata_database.cpp
)

target_include_directories(firelight_db
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PRIVATE ${CMAKE_SOURCE_DIR}/src/app
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(firelight_db
        PUBLIC Qt6::Core
        PUBLIC Qt6::Sql
        PRIVATE spdlog::spdlog
)
