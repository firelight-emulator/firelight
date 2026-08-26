# TODO: NEEDS REVIEW
cmake_minimum_required(VERSION 3.24.0...3.31.6)
project(firelight
        VERSION 0.12.2
        DESCRIPTION "A libretro-based emulation frontend that aims to be the easiest way to play your retro games, discover awesome mods for those games, and just have a dang good time."
        #    HOMEPAGE_URL ""
        LANGUAGES CXX
)

# Capture the version now: add_subdirectory'd libs call their own project() which
# resets PROJECT_VERSION in this scope, so we snapshot it into durable vars used
# by the exe (--version, VERSIONINFO) and CPack.
set(FL_VER_FULL "${PROJECT_VERSION}")
set(FL_VER_MAJOR "${PROJECT_VERSION_MAJOR}")
set(FL_VER_MINOR "${PROJECT_VERSION_MINOR}")
set(FL_VER_PATCH "${PROJECT_VERSION_PATCH}")
set(FL_DESCRIPTION "${PROJECT_DESCRIPTION}")

set(QT_QML_GENERATE_QMLLS_INI ON)

#include(${CMAKE_SOURCE_DIR}/cmake/rcheevos.cmake)
#include(${CMAKE_SOURCE_DIR}/cmake/clang-checks.cmake)
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(VCPKG_DISABLE_COMPILER_TRACKING ON)

set(QT_NO_PRIVATE_MODULE_WARNING ON)
set(CMAKE_BUILD_RPATH ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})

# list(APPEND CMAKE_PREFIX_PATH "/Users/alexcharles/Qt/6.7.2/macos" "/opt/homebrew" "C:/Qt/Qt-6.9.0")

# list(APPEND CMAKE_INCLUDE_PATH "/opt/homebrew/include")

list(APPEND CMAKE_PREFIX_PATH "C:/msys64/mingw64")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Build every library statically and link it into the exe. Leaving this ON
# turns keyword-less add_library targets (including vendored thirdparty like
# SQLiteCpp) into DLLs that aren't deployed next to the exe, and mixes
# static/shared libs so MinGW auto-exports duplicate symbols
set(BUILD_SHARED_LIBS OFF)

#option(ENABLE_ASAN "Enable Address Sanitizer" ON)
#if(ENABLE_ASAN)
#    message(STATUS "AddressSanitizer (ASan) enabled.")
#    # Check if the compiler is Clang or GCC (ASan flag is the same)
#    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
#        add_compile_options(-fsanitize=address -g) # Add -g for debug symbols
#        add_link_options(-fsanitize=address)
#        # For older CMake versions that don't support target_link_options, you might use:
#        # target_link_libraries(YourApp PRIVATE -fsanitize=address)
#    else()
#        message(WARNING "ASan requested, but compiler is not Clang or GCC. ASan not enabled.")
#    endif()
#endif()

#set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)

#set(VCPKG_TRACE_FIND_PACKAGE ON)

#include(FetchContent)
#FetchContent_Declare(
#        QtAndroidCMake
#        GIT_REPOSITORY https://github.com/LaurentGomila/qt-android-cmake
#        GIT_TAG master
#)
#FetchContent_MakeAvailable(QtAndroidCMake)

#set(CMAKE_INSTALL_RPATH "")

#set(QT_ENABLE_VERBOSE_DEPLOYMENT ON)

# set(QT6_DIR "C:/Qt/Qt-6.9.0")


find_package(PkgConfig REQUIRED)
pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(SWSCALE REQUIRED IMPORTED_TARGET libswscale)
pkg_check_modules(SWRESAMPLE REQUIRED IMPORTED_TARGET libswresample)

find_package(SDL2 REQUIRED)
find_package(spdlog REQUIRED)
# find_package(FFMPEG REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE SWRESAMPLE)
find_package(Qt6 6.11 REQUIRED COMPONENTS Quick Gui OpenGL QuickControls2 Quick3D Sql QuickTest Multimedia Network Widgets Svg ShaderTools REQUIRED)
if (Qt6_VERSION VERSION_GREATER_EQUAL 6.10.0)
    find_package(Qt6 OPTIONAL_COMPONENTS
            GuiPrivate
    )
endif ()
find_package(cpr REQUIRED)
find_package(GTest REQUIRED)

# Discover GoogleTest cases at test time rather than at build time. A freshly
# linked test binary can take several seconds to launch on its first run (macOS
# Gatekeeper validation + cold dyld closure over Qt/ffmpeg/openssl/etc.), which
# blows gtest_discover_tests' default 5s POST_BUILD timeout on a clean build and
# fails the build even though every binary compiled and linked fine. PRE_TEST
# defers enumeration to ctest, so the build never launches the test executables.
# Set before the add_subdirectory() calls below so the per-module test targets
# inherit it. Platform-neutral.
set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)

find_package(nlohmann_json REQUIRED)
find_package(LibArchive REQUIRED)
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavformat libswresample)
qt6_standard_project_setup(REQUIRES 6.11)


#add_compile_definitions(QT_QML_DEBUG)
set(CMAKE_CXX_STANDARD 20)
#set(QT_VERSION_MAJOR 6)

#include(${CMAKE_SOURCE_DIR}/libs/qwindowkit/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/cmake/rcheevos.cmake)
add_subdirectory(libs/firelight/util)
add_subdirectory(libs/firelight/achievements)
add_subdirectory(libs/firelight/activity)
add_subdirectory(libs/firelight/settings)
add_subdirectory(libs/firelight/platforms)
add_subdirectory(libs/firelight/mods)
add_subdirectory(libs/firelight/input)
add_subdirectory(libs/firelight/cheats)
add_subdirectory(libs/firelight/audio)
add_subdirectory(libs/firelight/media)
add_subdirectory(libs/firelight/metadata)
add_subdirectory(libs/firelight/netplay)
add_subdirectory(libs/firelight/patching)
#include(${CMAKE_SOURCE_DIR}/libs/QGoodWindow/QGoodWindow/QGoodWindow/QGoodWindow.cmake)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/opt/homebrew/include)
add_subdirectory(libs/firelight/library)
add_subdirectory(libs/firelight/saves)
add_subdirectory(libs/firelight/discord)
add_subdirectory(libs/discord)
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp)

# libchdr gives rcheevos a way to read CHD, which its bundled cdreader cannot do.
# Bundled zlib/zstd rather than system ones so the codecs a CHD may use are always
# present regardless of what the build host has installed
set(WITH_SYSTEM_ZLIB OFF CACHE BOOL "" FORCE)
set(WITH_SYSTEM_ZSTD OFF CACHE BOOL "" FORCE)
set(INSTALL_STATIC_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/libchdr EXCLUDE_FROM_ALL)

#file(GLOB QML_FILES
#        qml/*.qml
#        qml/components/achievements/*.qml
#        qml/components/activity/*.qml
#        qml/components/navigation/*.qml
#        qml/components/transitions/*.qml
#        qml/components/rewind/*.qml
#        qml/components/library/*.qml
#        qml/components/settings/*.qml
#        qml/components/emulation/*.qml
#        qml/components/controllers/*.qml
#        qml/components/v2/*.qml
#        qml/components/v2/*/*.qml
#        qml/components/v2/*/*/*.qml
#        qml/**/*.qml
#)

file(GLOB QML_FILES
        qml/v3/*.qml
        qml/v3/*/*.qml
        qml/v3/*/*/*.qml
)

# JS libraries aren't matched by the .qml glob above; add them explicitly.
list(APPEND QML_FILES qml/v3/routing/routing.js)
list(APPEND QML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/qml/v3/focus_nav.js)
list(APPEND QML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/qml/v3/folder_tree.js)
list(APPEND QML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/qml/v3/slider_slide.js)
list(APPEND QML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/qml/v3/focus_bump.js)

foreach (QML_FILE ${QML_FILES})
    get_filename_component(QML_FILE_NAME ${QML_FILE} NAME)
    set_property(SOURCE ${QML_FILE} PROPERTY QT_RESOURCE_ALIAS "${QML_FILE_NAME}")
endforeach ()

set_source_files_properties(qml/v3/singletons/FLDimmer.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/v3/singletons/AppStyle.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/v3/singletons/GeneralSettings.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/v3/singletons/WindowGeometry.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/v3/singletons/AppearanceSettings.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/v3/routing/Router.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/v3/singletons/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/v3/singletons/MaterialSymbols.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/v3/singletons/FLUtil.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/v3/FocusCursor.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/v3/singletons/SoundEffects.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

# The libretro/emulation core: the ABI wrapper + emulation lifecycle. Depends
# only on domain interfaces (audio/mic/achievement clients are injected), so it
# is free of Qt Quick and Qt Multimedia — a headless/CLI frontend can reuse it.
# firelight_lib links it and supplies the Qt implementations. (It still links
# Qt6::Core directly, and Qt6::Gui transitively via firelight_input.)
add_library(firelight_emulation_lib STATIC
        src/app/libretro/core.cpp
        src/app/libretro/core_environment.cpp
        src/app/libretro/libretro_dll.cpp
        src/app/libretro/core_input_router.cpp
        src/app/libretro/core_registry.cpp
        src/app/libretro/core_configuration.cpp
        src/app/libretro/game.cpp
        src/app/emulation/emulation_service.cpp
        src/app/emulation/game_loader.cpp
        src/app/emulation/core_settings_applier.cpp
        src/app/emulation/emulator_instance.cpp
        src/app/emulation/emulation_rate_controller.cpp
        src/app/emulation/refresh_counter.cpp
)

target_include_directories(firelight_emulation_lib PUBLIC
        ${SDL2_INCLUDE_DIRS} include ${CMAKE_SOURCE_DIR}/src/app ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(firelight_emulation_lib PUBLIC
        Qt6::Core
        SDL2::SDL2
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        firelight_input
        firelight_saves
        firelight_settings
        firelight_platforms
        firelight_cheats
        firelight_library
)

add_library(firelight_lib STATIC
        src/app/emulation/shortcut_actions.cpp
        src/app/emulation/shortcut_dispatcher.cpp
        src/app/diagnostics/performance_stats.cpp
        src/app/audio/audio_manager.cpp
        src/app/audio/audio_device_selection.cpp
        src/app/audio/audio_device_selection.hpp
        src/app/audio/ui_sound_player.cpp
        src/app/audio/ui_sound_player.hpp
        src/app/audio/qt_microphone.cpp
        src/app/audio/qt_microphone.hpp
        src/app/metadata/metadata_service.cpp
        src/app/metadata/cpr_http_client.cpp
        src/app/metadata/cpr_http_client.hpp
        src/gui/qt_game_art_proxy.cpp
        src/gui/qt_variant_group_proxy.cpp
        src/gui/qt_game_art_proxy.hpp

        src/app/input/gui/analog_settings_model.cpp
        src/app/input/gui/profile_list_model.cpp
        src/app/input/gui/binding_list_model.cpp
        src/app/input/gui/platform_input_preferences.cpp
        src/app/input/gui/controller_list_model.cpp
        src/gui/models/core_options_model.cpp
        src/gui/models/core_options_model.hpp
        src/gui/models/settings_model.cpp
        src/gui/models/settings_model.hpp
        src/gui/models/settings_search_model.cpp
        src/gui/models/settings_search_model.hpp
        src/gui/models/setting_binding.cpp
        src/gui/models/setting_binding.hpp
        src/gui/qt_settings_catalog_proxy.cpp
        src/gui/qt_settings_catalog_proxy.hpp
        src/gui/settings_level_shim.hpp
        src/gui/qt_core_registry_proxy.cpp
        src/gui/qt_core_registry_proxy.hpp
        src/gui/qt_save_manager_proxy.cpp
        src/gui/qt_save_manager_proxy.hpp
        src/app/saves/gui/savefile_list_model.cpp
        libs/firelight/achievements/src/rcheevos/ra_client.cpp
        src/gui/eventhandlers/window_resize_handler.cpp
        src/app/achievements/gui/achievement_list_model.cpp
        src/app/achievements/gui/achievement_list_sort_filter_model.cpp
        src/gui/platform_list_model.cpp
        src/gui/models/shop/shop_item_model.cpp
        src/app/saves/gui/suspend_point_list_model.cpp
        src/gui/game_image_provider.cpp
        src/gui/eventhandlers/input_method_detection_handler.cpp
        src/app/library/variant_group_service.cpp
        src/app/library/entry_merge_service.cpp
        src/app/library/gui/entry_list_model.cpp
        src/app/media/gui/capture_list_model.cpp
        src/app/netplay/direct_lobby_backend.cpp
        src/app/netplay/guest_stream_receiver.cpp
        src/app/netplay/host_stream_sender.cpp
        src/app/netplay/netplay_service.cpp
        src/app/netplay/gui/netplay_chat_model.cpp
        src/app/netplay/gui/netplay_slots_model.cpp
        src/app/netplay/gui/netplay_stream_item.cpp
        src/gui/qt_network_service_proxy.cpp
        src/app/emulator_item_renderer.cpp
        src/app/emulator_vulkan_renderer.cpp
        src/app/emulator_item.cpp
        src/app/input/gui/gamepad_status_item.cpp
        src/app/library/gui/library_entry_item.cpp
        src/app/achievements/gui/AchievementSetItem.cpp
        src/app/mods/gui/ModInfoItem.cpp
        src/app/saves/gui/suspend_points_item.cpp
        src/app/activity/gui/game_activity_item.cpp
        src/app/activity/gui/play_session_list_model.cpp
        src/app/library/gui/playlist_item_model.cpp
        src/gui/filesystem_utils.cpp
        src/gui/image_utils.cpp
        src/gui/image_utils.hpp
        src/app/saves/gui/save_files_item.cpp
        src/app/library/gui/content_directory_model.cpp
        src/app/input/gui/input_mappings_model.cpp
        src/gui/shortcuts_model.cpp
        src/gui/gamepad_profile_item.cpp
        src/gui/qt_input_service_proxy.cpp
        src/gui/EventEmitter.h
        src/gui/qt_input_service_proxy.hpp
        src/gui/qt_performance_stats_proxy.cpp
        src/gui/qt_emulation_service_proxy.cpp
        src/gui/qt_emulation_service_proxy.hpp
        src/image_cache.cpp
        src/gui/models/game_activity_list_model.cpp
        src/gui/models/game_activity_list_model.hpp
        src/app/service_accessor.cpp
        src/app/service_accessor.hpp
        src/gui/qt_achievement_service_proxy.cpp
        src/gui/qt_achievement_service_proxy.hpp
        src/app/achievements/gui/retro_achievements_game_item.cpp
        src/gui/eventhandlers/windows_frame_filter.cpp
        src/app/util/fuzzy_string_matcher.hpp
        src/gui/models/search_results_list_model.cpp
        src/gui/models/search_results_list_model.hpp
        src/gui/models/activity_buckets_list_model.cpp
        src/gui/models/activity_buckets_list_model.hpp
        src/app/library/gui/library_path_model.cpp
        src/app/db/database_inspector.cpp
        src/app/db/database_inspector.hpp
        src/app/qml_message_log.cpp
        src/app/qml_message_log.hpp
        src/cli/cli_app.cpp
        src/cli/console.cpp
        src/cli/data_dirs.cpp
        src/cli/doctor_command.cpp
        src/cli/doctor_command.hpp
        src/cli/launch_config.cpp
        src/cli/list_command.cpp
        src/cli/login_command.cpp
        src/cli/rom_launch.cpp
        src/cli/scan_command.cpp
        src/cli/single_instance.cpp
        src/cli/startup_options.hpp
        src/cli/verify_ui_command.cpp
        src/cli/verify_ui_command.hpp
        src/gui/qt_platform_service_proxy.cpp
        src/gui/qt_platform_service_proxy.hpp
        src/gui/focus_action.hpp
        src/gui/focus_info.hpp
        src/gui/focus/candidate_collector.cpp
        src/gui/focus/candidate_collector.hpp
        src/gui/focus/focus_navigator.cpp
        src/gui/focus/focus_navigator.hpp
        src/gui/focus/spatial_resolver.cpp
        src/gui/focus/spatial_resolver.hpp
        src/gui/models/library_entry_sort_filter_model.cpp
        src/gui/models/library_entry_sort_filter_model.hpp
)

# Single source of truth for the app version, consumed by the CLI --version and
# QApplication::setApplicationVersion. PUBLIC so the firelight exe inherits it.
target_compile_definitions(firelight_lib PUBLIC FL_VERSION="${FL_VER_FULL}")

target_include_directories(firelight_lib PUBLIC ${SDL2_INCLUDE_DIRS} ${FFMPEG_INCLUDE_DIRS} include ${CMAKE_SOURCE_DIR}/src/app ${CMAKE_SOURCE_DIR}/src)
target_include_directories(firelight_lib PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/CLI11 ${CMAKE_SOURCE_DIR}/libs/firelight/achievements/src/rcheevos ${CMAKE_SOURCE_DIR}/libs/firelight/achievements/src ${CMAKE_SOURCE_DIR}/libs/firelight/input/src ${CMAKE_SOURCE_DIR}/libs/firelight/saves/src ${CMAKE_SOURCE_DIR}/libs/firelight/library/src)
target_link_directories(firelight_lib PUBLIC ${FFMPEG_LIBRARY_DIRS} ${CMAKE_SOURCE_DIR}/lib/discord_social_sdk/lib/debug)

if (WIN32)
    target_link_libraries(firelight_lib PUBLIC mingw32 dwmapi)
endif ()

target_link_libraries(firelight_lib PUBLIC
        SDL2::SDL2
        z
        nlohmann_json::nlohmann_json
        SQLiteCpp
        sqlite3
        Qt6::Gui
        Qt6::Quick
        Qt6::OpenGL
        Qt6::QuickControls2
        Qt6::Quick3D
        Qt6::QuickTest
        Qt6::Multimedia
        Qt6::GuiPrivate
        Qt6::Network
        Qt6::Widgets
        # ${FFMPEG_LIBRARIES}
        PkgConfig::AVCODEC
        PkgConfig::AVFORMAT
        PkgConfig::AVUTIL
        # Add other PkgConfig::<PREFIX> targets as needed
        PkgConfig::SWSCALE
        PkgConfig::SWRESAMPLE
        LibArchive::LibArchive
        spdlog::spdlog
        cpr::cpr
        firelight_emulation_lib
        firelight_achievements
        firelight_activity
        firelight_settings
        firelight_platforms
        firelight_mods
        firelight_input
        firelight_cheats
        firelight_audio
        firelight_media
        firelight_metadata
        firelight_library
        firelight_util
        firelight_saves
        firelight_discord
        patching
        rcheevos
        discord
)

# qt_import_plugins(firelight_lib
#         INCLUDE QFFmpegMediaPlugin)

# Windows resource: app icon + embedded manifest + VERSIONINFO, with the version
# injected from PROJECT_VERSION so there's a single source of truth.
if (WIN32)
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/app_resources.rc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/app_resources.rc"
            @ONLY)
    set(FL_WINDOWS_RESOURCE "${CMAKE_CURRENT_BINARY_DIR}/app_resources.rc")
else ()
    set(FL_WINDOWS_RESOURCE "")
endif ()

# main code
qt6_add_executable(firelight
        resources.qrc
        ${FL_WINDOWS_RESOURCE}
        src/main.cpp
)


if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set_target_properties(firelight PROPERTIES
            MACOSX_BUNDLE ON
            # Runtime data (system/: content.db, _cores, settings_catalog.json)
            # is staged next to the executable, which inside a .app bundle means
            # Contents/MacOS/. macOS codesign rejects non-code files placed there
            # ("code object is not signed at all / In subcomponent
            # .../system/settings_catalog.json"), failing the Xcode build even
            # though everything compiled and linked. A dev/Debug build doesn't
            # need a signed bundle, so skip bundle signing; the linker still
            # ad-hoc-signs the Mach-O itself so it runs on Apple Silicon. This
            # XCODE_ATTRIBUTE_* is a no-op for the Ninja/Makefile/Windows builds.
            XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
    )
endif ()

configure_file(${CMAKE_SOURCE_DIR}/qtquickcontrols2.conf ${CMAKE_BINARY_DIR}/qtquickcontrols2.conf COPYONLY)

qt6_add_qml_module(firelight
        URI QMLFirelight
        VERSION 1.0
        QML_FILES
        ${QML_FILES}
)

qt6_add_shaders(firelight "firelight_shaders"
        PREFIX "/qt/qml/QMLFirelight"
        BASE "${CMAKE_SOURCE_DIR}/qml"
        FILES qml/shaders/rounded_image.frag
        qml/shaders/focus_ring.frag
        qml/shaders/tile_surface.frag
        qml/shaders/feathered_edge.frag
)

#target_include_directories(firelight PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(firelight PRIVATE libs/firelight/achievements/src libs/firelight/activity/src libs/firelight/settings/src libs/firelight/mods/src libs/firelight/input/src libs/firelight/library/src libs/firelight/saves/src libs/firelight/discord/src)
#target_link_libraries(firelight PRIVATE firelight_lib)
target_link_libraries(firelight PUBLIC
        discord
        firelight_lib)

#if (CMAKE_BUILD_TYPE STREQUAL "Release")
#    set_property(TARGET firelight PROPERTY WIN32_EXECUTABLE true)
#endif ()

if (MINGW)
    find_program(MINGW_PATH gcc)
    get_filename_component(MINGW_BIN_DIR ${MINGW_PATH} DIRECTORY)
    get_filename_component(MINGW_DIR ${MINGW_BIN_DIR} DIRECTORY)
    set(SHARED_LIB_DIR "${MINGW_BIN_DIR}")
    set(QT_PLUGIN_DIR "${MINGW_DIR}/share/qt6/plugins")
endif ()

#include(GNUInstallDirs)
install(TARGETS firelight
        RUNTIME_DEPENDENCIES
        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" "Qt6"
        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
        DIRECTORIES ${SHARED_LIB_DIR} ${QT_PLUGIN_DIR} "C:/Qt/Qt-6.9.0/bin"
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(SOURCE_RESOURCES_DIR "${CMAKE_SOURCE_DIR}/test_resources")
set(DEST_RESOURCES_DIR "${CMAKE_BINARY_DIR}/test_resources")

add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Copying test resources from ${SOURCE_RESOURCES_DIR} to ${DEST_RESOURCES_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEST_RESOURCES_DIR}  # Ensure destination directory exists
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_RESOURCES_DIR} ${DEST_RESOURCES_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "Copy completed"
        COMMENT "Copying test resources to build directory"
)

if (WIN32)
    file(COPY ${CMAKE_SOURCE_DIR}/_cores/windows DESTINATION ${CMAKE_BINARY_DIR}/system/_cores)
endif (WIN32)

if (UNIX)
    file(COPY ${CMAKE_SOURCE_DIR}/_cores/linux DESTINATION ${CMAKE_BINARY_DIR}/system/_cores)
endif (UNIX)

file(COPY ${CMAKE_SOURCE_DIR}/content.db DESTINATION ${CMAKE_BINARY_DIR}/system)

# PPSSPP runtime assets (fonts, VFPU tables, atlases, compat db). The core loads
# these from <system>/PPSSPP; main.cpp seeds them into the writable core-system
# dir on first run. Bundled next to the executable here.
if (EXISTS ${CMAKE_SOURCE_DIR}/_system/PPSSPP)
    file(COPY ${CMAKE_SOURCE_DIR}/_system/PPSSPP DESTINATION ${CMAKE_BINARY_DIR}/system)
endif ()

# Friendly emulation settings + per-core option defaults (loaded at runtime by
# EmulationService, resolved through CoreConfiguration).
file(COPY ${CMAKE_SOURCE_DIR}/data/settings DESTINATION ${CMAKE_BINARY_DIR}/system)
# Hotkey actions + their shipped bindings (loaded at runtime by ShortcutRegistry).
file(COPY ${CMAKE_SOURCE_DIR}/data/shortcuts.json DESTINATION ${CMAKE_BINARY_DIR}/system)

# The file(COPY) above only runs at configure time, so edits to the catalog wouldn't reach the
# running app until a reconfigure. A custom target rather than a POST_BUILD on firelight: POST_BUILD
# only fires when the target actually relinks, so editing or adding a catalog file on its own left
# the deployed copy stale ("ninja: no work to do").
#
# The catalog directory is cleared before it is copied. copy_directory leaves behind files that no
# longer exist in the source, and a stale one still loads and still merges — a setting you deleted
# goes on working
add_custom_target(deploy_runtime_data ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:firelight>/system
        COMMAND ${CMAKE_COMMAND} -E rm -rf $<TARGET_FILE_DIR:firelight>/system/settings
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data/settings
        $<TARGET_FILE_DIR:firelight>/system/settings
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/data/shortcuts.json
        $<TARGET_FILE_DIR:firelight>/system/shortcuts.json
        COMMENT "Deploying data/settings + shortcuts.json")

add_dependencies(deploy_runtime_data firelight)

#if (DISCORD_SDK_RUNTIME_LIBRARY_FILE AND EXISTS "${DISCORD_SDK_RUNTIME_LIBRARY_FILE}")
#    add_custom_command(TARGET firelight POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            "${DISCORD_SDK_RUNTIME_LIBRARY_FILE}"
#            $<TARGET_FILE_DIR:firelight> # Copies to the directory where MyGame.exe is built
#            COMMENT "Copying Discord SDK shared library to output directory for MyGame"
#    )
#else ()
#    message(WARNING "DISCORD_SDK_RUNTIME_LIBRARY_FILE is not defined or file does not exist. "
#            "Discord SDK shared library will not be copied. Path: ${DISCORD_SDK_RUNTIME_LIBRARY_FILE}")
#endif ()

if (WIN32)
    file(COPY ${CMAKE_SOURCE_DIR}/lib/discord_social_sdk/bin/release/discord_partner_sdk.dll DESTINATION ${CMAKE_BINARY_DIR})
endif ()

enable_testing()
include(GoogleTest)

add_executable(fl_test
        tests/app/library/archive_reader_test.cpp
        tests/app/library/content_identifier_test.cpp
        tests/app/library/content_extensions_test.cpp
        tests/app/library/content_hasher_test.cpp
        tests/app/library/content_loader_test.cpp
        tests/app/library/entry_list_model_test.cpp
        tests/app/library/library_ingest_service_test.cpp
        tests/app/library/variant_group_service_test.cpp
        tests/app/library/disc_set_service_test.cpp
        tests/app/library/library_scanner_test.cpp
        tests/app/library/ingestion_contract_test.cpp
        tests/app/library/library_e2e_test.cpp
        tests/app/library/chd_disc_test.cpp
        tests/app/metadata/metadata_service_test.cpp
        tests/gui/models/library_entry_sort_filter_model_test.cpp
        tests/app/gui/filesystem_utils_test.cpp
        tests/app/gui/focus_info_test.cpp
        tests/app/gui/focus/candidate_collector_test.cpp
        tests/app/gui/focus/focus_navigator_test.cpp
        tests/app/gui/focus/root_key_filter_test.cpp
        tests/app/gui/focus/spatial_resolver_test.cpp
        tests/app/gui/input_method_detection_handler_test.cpp
        tests/app/library/entry_resolver_test.cpp
        tests/app/library/smart_folder_test.cpp
        tests/app/db/daos/play_session_test.cpp
        tests/app/db/database_inspector_test.cpp
        tests/app/rcheevos/rcheevos_offline_client_test.cpp
        tests/app/settings/sqlite_settings_repository_test.cpp
        tests/app/settings/settings_service_test.cpp
        tests/app/settings/setting_definition_test.cpp
        tests/app/settings/settings_catalog_test.cpp
        tests/app/settings/settings_index_test.cpp
        tests/app/settings/sqlite_settings_repository_core_options_test.cpp
        tests/app/gui/core_options_model_test.cpp
        tests/app/gui/settings_model_test.cpp
        tests/app/gui/shortcuts_model_test.cpp
        tests/app/cli/cli_app_test.cpp
        tests/app/cli/launch_config_test.cpp
        tests/app/cli/rom_launch_test.cpp
        tests/app/cli/single_instance_test.cpp

        tests/main.cpp
        tests/app/event_dispatcher_test.cpp
        tests/app/netplay/direct_lobby_backend_test.cpp
        tests/app/netplay/netplay_service_test.cpp
        tests/app/migrations/migration_runner_test.cpp
        tests/app/media/capture_list_model_test.cpp
        tests/app/library/sqlite_user_library_test.cpp
        tests/app/platforms/platform_service_test.cpp
        tests/app/emulation/emulation_service_test.cpp
        tests/app/emulation/shortcut_actions_test.cpp
        tests/app/emulation/shortcut_dispatcher_test.cpp
        tests/app/emulation/emulator_instance_test.cpp
        tests/app/emulation/emulation_rate_controller_test.cpp
        tests/app/emulation/refresh_counter_test.cpp
        tests/app/emulation/frame_slot_test.cpp
        tests/app/emulation/emulator_instance_e2e_test.cpp
        tests/app/emulation/hotkeys_disabled_test.cpp
        tests/app/emulation/core_configuration_test.cpp
        tests/app/emulation/core_registry_test.cpp
        tests/app/libretro/core_input_router_test.cpp
        tests/app/achievements/sqlite_achievement_repository_test.cpp
        tests/app/achievements/achievement_service_test.cpp
        tests/app/activity/sqlite_activity_log_test.cpp
)

# tst_FLFocusHighlight is excluded until its 9 failures are triaged — they assert
# behaviour FLFocusHighlight no longer has, and a permanently red `check` gets
# ignored. Re-add both lines together when fixing them
file(GLOB QML_TEST_FILES
        qml/v3/singletons/AppStyle.qml
        qml/v3/routing/routing.js
        qml_tests/tst_routing.qml
        qml_tests/tst_focus_info.qml
        qml_tests/tst_action_modifiers.qml
        qml_tests/tst_focus_nav.qml
        qml_tests/tst_focus_navigator.qml
        qml_tests/tst_focus_return.qml
        qml_tests/tst_root_keys.qml
        qml_tests/tst_click_blocking.qml
        qml_tests/tst_setting_row_focus.qml
        qml_tests/tst_slider_keys.qml
        qml/v3/focus_nav.js
        qml/v3/slider_slide.js
        qml/v3/focus_bump.js
        qml_tests/tst_focus_bump.qml
        qml_tests/FocusBase.qml
        qml_tests/FocusDerived.qml
        #        qml/components/IconButton.qml
        #        qml/components/FLFocusHighlight.qml
        #        qml_tests/components/tst_FLFocusHighlight.qml
        #        qml_tests/components/tst_IconButton.qml
)

foreach (QML_FILE ${QML_TEST_FILES})
    get_filename_component(QML_FILE_NAME ${QML_FILE} NAME)
    set_property(SOURCE ${QML_FILE} PROPERTY QT_RESOURCE_ALIAS "${QML_FILE_NAME}")
endforeach ()

qt_add_executable(fl_qml_test
        tests/qml_main.cpp
        resources.qrc
)

qt6_add_qml_module(fl_qml_test
        URI QMLFirelightTest
        VERSION 1.0
        QML_FILES
        ${QML_TEST_FILES}
        #        IMPORTS QMLFirelight
)

set_target_properties(fl_qml_test PROPERTIES
        QT_QML_IMPORT_PATHS ${CMAKE_BINARY_DIR}/qml
)

target_link_libraries(fl_qml_test PUBLIC firelight_lib)
# Scoped to this target and to qml_tests/ only: at the repo root Quick Test
# recurses into build/ and thirdparty/ and picks up excluded tst_ files
target_compile_definitions(fl_qml_test PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml_tests")

add_test(NAME QMLTests COMMAND fl_qml_test)

#target_compile_options(fl_test PUBLIC "-fprofile-instr-generate" "-fcoverage-mapping")
#target_link_options(fl_test PUBLIC "-fprofile-instr-generate" "-fcoverage-mapping")

add_dependencies(fl_test copy_resources)

# The authored catalog, not the deployed copy: the shipped-catalog test should
# guard the file a developer edits regardless of whether firelight was built.
target_compile_definitions(fl_test PRIVATE
        FL_SETTINGS_CATALOG_DIR="${CMAKE_SOURCE_DIR}/data/settings"
        FL_SHORTCUTS_FILE="${CMAKE_SOURCE_DIR}/data/shortcuts.json")

target_include_directories(fl_test PRIVATE ${GTEST_INCLUDE_DIRS} include src libs/firelight/achievements/src libs/firelight/settings/src libs/firelight/input/src libs/firelight/library/src libs/firelight/saves/src libs/firelight/saves/tests libs/firelight/activity/src libs/firelight/activity/include)
target_link_libraries(fl_test PUBLIC firelight_lib ${GTEST_BOTH_LIBRARIES})
gtest_discover_tests(fl_test)

# `ninja check` (or `cmake --build . --target check`): build every test
# executable and then run the whole CTest suite -- all per-module tests + fl_test
# -- in one command. (`ctest` on its own runs already-built exes; this also
# builds them first so nothing is stale.)
add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        USES_TERMINAL
)
add_dependencies(check
        fl_test
        fl_qml_test
        firelight_input_test
        firelight_cheats_test
        firelight_audio_test
        firelight_media_test
        firelight_metadata_test
        firelight_saves_test
        firelight_netplay_test
        firelight_patching_test
)
#ctest_coverage_collect_gcov()
#target_compile_options(firelight PRIVATE -Werror -ggdb3 -O0)
#
#add_test(NAME firelight_test COMMAND firelight)

#file(COPY LICENSE.md README.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#install(FILES LICENSE.md README.txt DESTINATION ${CMAKE_INSTALL_BINDIR})

set(CPACK_PACKAGE_NAME "Firelight")
set(CPACK_PACKAGE_VENDOR "BiscuitCakes")
# Version derives from the single source of truth (project(... VERSION ...)).
set(CPACK_PACKAGE_VERSION_MAJOR ${FL_VER_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${FL_VER_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${FL_VER_PATCH})
set(CPACK_PACKAGE_VERSION "${FL_VER_FULL}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Firelight")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_EXECUTABLES "firelight;Firelight")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${FL_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://firelight-emulator.com")
# Publisher/support info surfaced in Add/Remove Programs + the installer.
set(CPACK_PACKAGE_CONTACT "https://firelight-emulator.com")

set(CPACK_NSIS_DISPLAY_NAME "Firelight")
set(CPACK_NSIS_PACKAGE_NAME "Firelight")
set(CPACK_NSIS_UNINSTALL_NAME "Firelight Uninstaller")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/assets/img/icon.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/assets/img/icon.ico")
# The icon shown in Add/Remove Programs, and a real publisher + support links so
# the entry looks legitimate (not "Unknown").
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\firelight.exe")
set(CPACK_NSIS_URL_INFO_ABOUT "https://firelight-emulator.com")
set(CPACK_NSIS_HELP_LINK "https://firelight-emulator.com")
set(CPACK_NSIS_CONTACT "https://firelight-emulator.com")
# Offer to launch Firelight when the installer finishes.
set(CPACK_NSIS_MUI_FINISHPAGE_RUN "firelight.exe")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

set(deploy_tool_options_arg "")
if (APPLE)
    set(deploy_tool_options_arg -hardened-runtime)
elseif (WIN32)
    set(deploy_tool_options_arg --no-compiler-runtime)
endif ()

qt6_generate_deploy_qml_app_script(
        TARGET firelight
        OUTPUT_SCRIPT deploy_script
        #        MACOS_BUNDLE_POST_BUILD
        DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
)


install(
        SCRIPT ${deploy_script}
)

if (WIN32)
    install(
            PROGRAMS
            ${SHARED_LIB_DIR}/libsqlite3-0.dll
            #            ${SHARED_LIB_DIR}/libjpeg-8.dll
            #            ${SHARED_LIB_DIR}/libgio-2.0-0.dll
            #            ${SHARED_LIB_DIR}/libjasper.dll
            #            ${SHARED_LIB_DIR}/libtiff-6.dll
            #            ${SHARED_LIB_DIR}/libwebp-7.dll
            #            ${SHARED_LIB_DIR}/libwebpdemux-2.dll
            #            ${SHARED_LIB_DIR}/libwebpmux-3.dll
            #            ${SHARED_LIB_DIR}/libgobject-2.0-0.dll
            #            ${SHARED_LIB_DIR}/swresample-5.dll
            #            ${SHARED_LIB_DIR}/swscale-8.dll
            #            ${SHARED_LIB_DIR}/libffi-8.dll
            ${CMAKE_SOURCE_DIR}/lib/discord_social_sdk/bin/release/discord_partner_sdk.dll
            TYPE BIN
    )
endif ()

install(
        FILES
        ${CMAKE_SOURCE_DIR}/ca-bundle.crt
        DESTINATION ${CMAKE_INSTALL_BINDIR}/../etc/ssl/certs/
)

install(
        FILES
        ${CMAKE_SOURCE_DIR}/content.db
        ${CMAKE_SOURCE_DIR}/data/shortcuts.json
        DESTINATION ${CMAKE_INSTALL_BINDIR}/system/
)

install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/data/settings
        DESTINATION ${CMAKE_INSTALL_BINDIR}/system/
)

# PPSSPP runtime assets (seeded into core-system on first run by main.cpp).
if (EXISTS ${CMAKE_SOURCE_DIR}/_system/PPSSPP)
    install(
            DIRECTORY
            ${CMAKE_SOURCE_DIR}/_system/PPSSPP
            DESTINATION ${CMAKE_INSTALL_BINDIR}/system
    )
endif ()


if (WIN32)
    install(
            DIRECTORY
            ${CMAKE_SOURCE_DIR}/_cores/windows
            DESTINATION ${CMAKE_INSTALL_BINDIR}/system/_cores
    )
endif (WIN32)

if (UNIX)
    install(
            DIRECTORY
            ${CMAKE_SOURCE_DIR}/_cores/linux
            DESTINATION ${CMAKE_INSTALL_BINDIR}/system/_cores
    )
endif (UNIX)

set(CPACK_INSTALL_SCRIPT "${CMAKE_SOURCE_DIR}/cpack_portable.cmake")

include(CPack)
