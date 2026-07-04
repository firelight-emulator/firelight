cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Core Gui)
find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(SWSCALE REQUIRED IMPORTED_TARGET libswscale)

add_library(firelight_media STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/clip_muxer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/clip_recorder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/media_service.cpp
)
target_include_directories(firelight_media
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(firelight_media
        PUBLIC Qt6::Core
        PUBLIC Qt6::Gui
        PRIVATE spdlog::spdlog
        PRIVATE PkgConfig::AVCODEC
        PRIVATE PkgConfig::AVFORMAT
        PRIVATE PkgConfig::AVUTIL
        PRIVATE PkgConfig::SWSCALE
)

# --- Unit tests (self-contained: link only firelight_media) ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_media_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/media_service_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/clip_recorder_test.cpp
)
target_include_directories(firelight_media_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
        PRIVATE ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(firelight_media_test
        PRIVATE firelight_media
        PRIVATE Qt6::Core
        PRIVATE Qt6::Gui
        PRIVATE GTest::gtest
        PRIVATE PkgConfig::AVFORMAT
        PRIVATE PkgConfig::AVCODEC
        PRIVATE PkgConfig::AVUTIL
)
gtest_discover_tests(firelight_media_test)
