cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Sql)
find_package(spdlog REQUIRED)
find_package(LibArchive REQUIRED)
find_package(ZLIB REQUIRED)

add_library(firelight_library STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_user_library.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/library_scanner2.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/rom_file.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/patch_file.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/firelight/library/sqlite_user_library.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/firelight/library/library_scanner2.hpp
)

target_include_directories(firelight_library
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PRIVATE ${CMAKE_SOURCE_DIR}/src/app
        PRIVATE ${CMAKE_SOURCE_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(firelight_library
        PUBLIC Qt6::Core
        PRIVATE Qt6::Concurrent
        PRIVATE Qt6::Sql
        PRIVATE spdlog::spdlog
        PRIVATE LibArchive::LibArchive
        PRIVATE ZLIB::ZLIB
        PRIVATE patching
        PRIVATE rcheevos
)
