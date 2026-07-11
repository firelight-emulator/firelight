cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

find_package(spdlog REQUIRED)

# Qt-free domain library: the save/suspend metadata index (SQLiteCpp) plus the
# filesystem-backed SaveManager. QML/Qt bridging lives in the app layer
# (QtSaveManagerProxy). AUTOMOC OFF because there are no QObjects here.
add_library(firelight_saves STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/save_manager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/savefile.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_save_database.cpp
)

set_target_properties(firelight_saves PROPERTIES AUTOMOC OFF)

target_include_directories(firelight_saves
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)

target_link_libraries(firelight_saves
        PRIVATE spdlog::spdlog
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
)

# --- Unit tests (self-contained: link only firelight_saves; no Qt) ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_saves_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/sqlite_save_database_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/save_manager_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/savefile_metadata_test.cpp
)
target_include_directories(firelight_saves_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)
target_link_libraries(firelight_saves_test
        PRIVATE firelight_saves
        PRIVATE GTest::gtest
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
        PRIVATE spdlog::spdlog
)
gtest_discover_tests(firelight_saves_test)
