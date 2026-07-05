cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Sql)
find_package(spdlog REQUIRED)

add_library(firelight_saves STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/save_manager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/savefile.cpp
)

# The module is QObject-free (notifications go through the EventDispatcher, and
# the QML save-directory binding lives in QtSaveManagerProxy). Turn off AUTOMOC
# (inherited ON from the parent) to avoid an empty-autogen dependency cycle.
set_target_properties(firelight_saves PROPERTIES AUTOMOC OFF)

target_include_directories(firelight_saves
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(firelight_saves
        PUBLIC Qt6::Core
        PUBLIC Qt6::Gui
        PRIVATE Qt6::Sql
        PRIVATE spdlog::spdlog
        PRIVATE firelight_db
)
