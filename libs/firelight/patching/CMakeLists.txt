cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

find_package(ZLIB REQUIRED)
find_package(spdlog REQUIRED)

add_library(patching STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/ips_patch.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pm_star_rod_mod_patch.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/yay_0_codec.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/ups_patch.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/bps_patch.cpp
)

# The module is Qt-free (no Q_OBJECT), so no moc is needed. Turn off AUTOMOC
# (inherited ON from the parent) to avoid an empty-autogen dependency cycle.
set_target_properties(patching PROPERTIES AUTOMOC OFF)

target_include_directories(patching
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(patching
        PRIVATE spdlog::spdlog
        PRIVATE ZLIB::ZLIB
)

# --- Unit tests (self-contained: link only patching) ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_patching_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/bps_patch_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/ips_patch_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/pm_star_rod_mod_patch_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/ups_patch_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/util_test.cpp
)
target_link_libraries(firelight_patching_test
        PRIVATE patching
        PRIVATE spdlog::spdlog
        PRIVATE GTest::gtest
)
# The patch tests read fixtures from test_resources/, staged into the top build
# dir; run them from there
gtest_discover_tests(firelight_patching_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
