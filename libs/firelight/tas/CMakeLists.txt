cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

# The in-app TAS harness core (roadmap Phase 1). Deliberately Qt-free and
# core-agnostic: it depends only on the header-only canonical input type
# (firelight::input::InputFrame) and the libretro IRetroPad/IRetropadProvider
# interfaces, so it links into the app *and* unit-tests headlessly with plain
# GoogleTest — no Qt Quick, no Vulkan, no SDL. The app-side glue that binds these
# to EmulatorInstance / the render-thread command queue lives in src/app.

add_library(firelight_tas STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/greenzone_store.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/tas_session.cpp
)
target_include_directories(firelight_tas
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        # Header-only, Qt-free dependencies: the canonical InputFrame and the
        # libretro pad interfaces (retropad.hpp lives under the repo include/ root).
        PUBLIC ${CMAKE_SOURCE_DIR}/libs/firelight/input/include
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# --- Unit tests (self-contained: no Qt, unlike firelight_input's suite) ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_tas_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/scripted_retropad_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/movie_input_provider_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/greenzone_store_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/tas_session_test.cpp
)
target_link_libraries(firelight_tas_test
        PRIVATE firelight_tas
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_tas_test)
