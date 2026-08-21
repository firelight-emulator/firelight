cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)
find_package(spdlog REQUIRED)
find_package(LibArchive REQUIRED)
find_package(ZLIB REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(firelight_library STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_user_library_repository.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/smart_folder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/metadata_overrides.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/filename_tags.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/game_identity.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/accepted_extensions.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/variant_group.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/library_scanner2.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/archive_reader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/content_hasher.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/content_identifier.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/content_loader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/chd_cdreader.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/rc_hash_logging.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/disc_inspector.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/disc_set.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/entry_status.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/disc_set_playlist.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/disc_set_service.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/entry_resolver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/library_ingest_service.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/user_library_service.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/patch_file.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/firelight/library/library_scanner2.hpp
)

target_include_directories(firelight_library
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PRIVATE ${CMAKE_SOURCE_DIR}/src/app
        PRIVATE ${CMAKE_SOURCE_DIR}/src
        PRIVATE ${CMAKE_SOURCE_DIR}/libs/rcheevos/src/rhash
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(firelight_library
        PUBLIC firelight_util
        PRIVATE chdr-static
        PUBLIC Qt6::Core
        PRIVATE Qt6::Concurrent
        PRIVATE SQLiteCpp
        PRIVATE spdlog::spdlog
        PRIVATE LibArchive::LibArchive
        PRIVATE ZLIB::ZLIB
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE patching
        PRIVATE rcheevos
        PRIVATE firelight_platforms
)

# --- Unit tests for the parts that need no database, no Qt and no services ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_library_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/metadata_overrides_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/filename_tags_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/disc_set_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/entry_status_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/disc_set_playlist_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/game_identity_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/variant_primary_test.cpp
)
target_include_directories(firelight_library_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
)
target_link_libraries(firelight_library_test
        PRIVATE firelight_library
        PRIVATE firelight_util
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_library_test)
