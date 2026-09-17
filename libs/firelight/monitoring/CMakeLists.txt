# TODO: NEEDS REVIEW
cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

add_library(firelight_monitoring STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/monitor.cpp
)
target_include_directories(firelight_monitoring
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

# --- Unit tests (self-contained: link only firelight_monitoring) ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_monitoring_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/trace_ring_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/monitor_test.cpp
)
target_include_directories(firelight_monitoring_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
)
target_link_libraries(firelight_monitoring_test
        PRIVATE firelight_monitoring
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_monitoring_test)
