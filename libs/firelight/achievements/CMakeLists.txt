cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 20)

add_library(firelight_achievements STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/achievement_service.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_achievement_repository.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/rcheevos/rcheevos_offline_client.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/rcheevos/regular_http_client.cpp
)

target_include_directories(firelight_achievements
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/include/rcheevos
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/rcheevos
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(firelight_achievements
        PRIVATE spdlog::spdlog
        PRIVATE cpr::cpr
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
        PRIVATE Qt6::Core
        PRIVATE rcheevos)