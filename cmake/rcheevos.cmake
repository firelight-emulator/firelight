cmake_minimum_required(VERSION 3.22.1)
project(rcheevos)

add_compile_definitions(RC_DISABLE_LUA)

set(CMAKE_CXX_STANDARD 20)

file(GLOB RCHEEVOS_SOURCES
        ${CMAKE_SOURCE_DIR}/libs/rcheevos/src/*.c
        ${CMAKE_SOURCE_DIR}/libs/rcheevos/src/**/*.c)

add_library(rcheevos STATIC ${RCHEEVOS_SOURCES})

target_include_directories(rcheevos PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/rcheevos/include
        ${CMAKE_SOURCE_DIR}/include/libretro)