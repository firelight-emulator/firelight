cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

find_package(nlohmann_json REQUIRED)

add_library(firelight_settings STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_settings_repository.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/settings_service.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/settings_catalog.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/settings_index.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/firelight/settings/sqlite_settings_repository.hpp
)

# The module is Qt-free (no Q_OBJECT), so no moc is needed. Turn off AUTOMOC
# (inherited ON from the parent) to avoid an empty-autogen dependency cycle.
set_target_properties(firelight_settings PROPERTIES AUTOMOC OFF)

target_include_directories(firelight_settings
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)

target_link_libraries(firelight_settings
        PRIVATE firelight_util
        PRIVATE spdlog::spdlog
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
        PRIVATE nlohmann_json::nlohmann_json
)
