cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

# WebRTC data channels for the peer-to-peer data plane. Media/websocket parts
# disabled — only data channels + ICE are used.
include(FetchContent)
set(NO_MEDIA ON CACHE BOOL "" FORCE)
set(NO_WEBSOCKET ON CACHE BOOL "" FORCE)
set(NO_EXAMPLES ON CACHE BOOL "" FORCE)
set(NO_TESTS ON CACHE BOOL "" FORCE)
# libdatachannel's vendored plog declares a pre-3.5 minimum newer CMake rejects.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_Declare(libdatachannel
        GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
        GIT_TAG v0.22.5
        GIT_SHALLOW ON
)
FetchContent_MakeAvailable(libdatachannel)

add_library(firelight_netplay STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/messages.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/protocol.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/rtc_transport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/session.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/signal_chunker.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/slot_table.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/stream_packets.cpp
)
target_include_directories(firelight_netplay
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(firelight_netplay
        PRIVATE spdlog::spdlog
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE LibDataChannel::LibDataChannelStatic
)

# --- Unit tests (self-contained: link only firelight_netplay) ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_netplay_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/messages_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/rtc_transport_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/session_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/signal_chunker_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/slot_table_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/stream_packets_test.cpp
)
target_include_directories(firelight_netplay_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
)
target_link_libraries(firelight_netplay_test
        PRIVATE firelight_netplay
        PRIVATE spdlog::spdlog
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_netplay_test)
