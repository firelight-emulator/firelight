cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED IMPORTED_TARGET libswresample)

add_library(firelight_audio STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/audio_rate_controller.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/audio_resampler.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/pcm_converter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/sfx_mixer.cpp
)
target_include_directories(firelight_audio
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(firelight_audio
        PRIVATE spdlog::spdlog
        PUBLIC PkgConfig::AVUTIL
        PUBLIC PkgConfig::SWRESAMPLE
)

# --- Unit tests (self-contained: link only firelight_audio) ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_audio_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/audio_rate_controller_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/audio_resampler_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/pcm_converter_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/sfx_mixer_test.cpp
)
target_include_directories(firelight_audio_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
)
target_link_libraries(firelight_audio_test
        PRIVATE firelight_audio
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_audio_test)
