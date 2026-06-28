cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

find_package(spdlog REQUIRED)

add_library(firelight_discord STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/discord_manager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/firelight/discord/discord_manager_impl.hpp
)

target_include_directories(firelight_discord
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PRIVATE ${CMAKE_SOURCE_DIR}/src/app
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(firelight_discord
        PUBLIC discord
        PRIVATE spdlog::spdlog
)
