find_package(spdlog REQUIRED)
cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

find_package(nlohmann_json REQUIRED)

add_library(firelight_platforms STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/platform_service.cpp
)

target_include_directories(firelight_platforms
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(firelight_platforms
        PUBLIC nlohmann_json::nlohmann_json
        PRIVATE spdlog::spdlog
)
