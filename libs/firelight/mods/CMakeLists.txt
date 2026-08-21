cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

add_library(firelight_mods STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_mod_repository.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/firelight/mods/sqlite_mod_repository.hpp
)

target_include_directories(firelight_mods
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(firelight_mods
        PUBLIC firelight_platforms
)
