cmake_minimum_required(VERSION 3.23.5...3.31.6)
project(firelight
        VERSION 0.11.0
        DESCRIPTION "A libretro-based emulation frontend that aims to be the easiest way to play your retro games, discover awesome mods for those games, and just have a dang good time."
        #    HOMEPAGE_URL ""
        LANGUAGES CXX
)

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
find_package(Qt6 6.8 REQUIRED COMPONENTS Quick Gui OpenGL QuickControls2 Quick3D Sql QuickTest Multimedia Network Widgets Svg REQUIRED)
find_package(cpr REQUIRED)
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(LibArchive REQUIRED)
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavformat libswresample)
qt6_standard_project_setup(REQUIRES 6.8)


#add_compile_definitions(QT_QML_DEBUG)
set(CMAKE_CXX_STANDARD 20)
#set(QT_VERSION_MAJOR 6)

#include(${CMAKE_SOURCE_DIR}/libs/qwindowkit/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/cmake/rcheevos.cmake)
#include(${CMAKE_SOURCE_DIR}/libs/QGoodWindow/QGoodWindow/QGoodWindow/QGoodWindow.cmake)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/opt/homebrew/include)
add_subdirectory(src/app)
add_subdirectory(libs/discord)
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp)

file(GLOB QML_FILES
        qml/*.qml
        qml/components/activity/*.qml
        qml/components/navigation/*.qml
        qml/components/transitions/*.qml
        qml/components/rewind/*.qml
        qml/components/library/*.qml
        qml/components/settings/*.qml
        qml/components/emulation/*.qml
        qml/components/controllers/*.qml
        qml/**/*.qml
)

foreach (QML_FILE ${QML_FILES})
    get_filename_component(QML_FILE_NAME ${QML_FILE} NAME)
    set_property(SOURCE ${QML_FILE} PROPERTY QT_RESOURCE_ALIAS "${QML_FILE_NAME}")
endforeach ()

set_source_files_properties(qml/Constants.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/ColorPalette.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/AppStyle.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/GeneralSettings.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/AppearanceSettings.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

add_library(firelight_lib
        src/app/libretro/core.cpp
        src/app/libretro/game.cpp
        src/app/db/sqlite_content_database.cpp
        src/app/input2/sdl/sdl_controller.cpp
        src/app/manager_accessor.cpp
        src/app/audio/audio_manager.cpp
        src/app/saves/save_manager.cpp
        src/app/saves/savefile.cpp
        src/app/db/sqlite_userdata_database.cpp
        src/app/input/gui/controller_list_model.cpp
        src/app/saves/gui/savefile_list_model.cpp
        src/app/rcheevos/ra_client.cpp
        src/gui/eventhandlers/window_resize_handler.cpp
        src/app/achievements/gui/achievement_list_model.cpp
        src/app/achievements/gui/achievement_list_sort_filter_model.cpp
        src/gui/router.cpp
        src/app/input/profiles/input_mapping.cpp
        src/gui/platform_list_model.cpp
        src/app/libretro/core_configuration.cpp
        src/app/emulator_config_manager.cpp
        src/gui/models/shop/shop_item_model.cpp
        src/app/saves/gui/suspend_point_list_model.cpp
        src/gui/game_image_provider.cpp
        src/app/audio/SfxPlayer.cpp
        src/app/rcheevos/regular_http_client.cpp
        src/app/rcheevos/offline/ra_cache.cpp
        src/app/rcheevos/offline/rcheevos_offline_client.cpp
        src/gui/eventhandlers/input_method_detection_handler.cpp
        src/app/input/keyboard_input_handler.cpp
        src/app/library/gui/entry_list_model.cpp
        src/app/emulator_item_renderer.cpp
        src/app/emulator_item.cpp
        src/app/input/gui/gamepad_status_item.cpp
        src/app/activity/sqlite_activity_log.cpp
        src/app/input/sqlite_controller_repository.cpp
        src/app/library/gui/library_entry_item.cpp
        src/later/video_encoder.cpp
        src/later/video_decoder.cpp
        src/app/achievements/gui/AchievementSetItem.cpp
        src/app/mods/gui/ModInfoItem.cpp
        src/app/mods/SqliteModRepository.cpp
        src/app/saves/gui/suspend_points_item.cpp
        src/app/activity/gui/game_activity_item.cpp
        src/app/activity/gui/play_session_list_model.cpp
        src/app/achievements/retroachievements_backend.cpp
        src/app/settings/sqlite_settings_repository.cpp
        src/app/discord/discord_manager.cpp
        src/app/library/gui/playlist_item_model.cpp
        src/app/library/gui/entry_sort_filter_list_model.cpp
        src/gui/filesystem_utils.cpp
        src/app/saves/gui/save_files_item.cpp
        src/app/library/gui/content_directory_model.cpp
        src/app/input/gui/input_mappings_model.cpp
        src/app/platforms/platform_service.cpp
        src/app/input/profiles/shortcut_mapping.cpp
        src/app/input2/input_service.cpp
        src/app/input2/gamepad_profile.cpp
        src/app/input2/sdl/sdl_input_service.cpp
        src/gui/shortcuts_model.cpp
        src/gui/gamepad_profile_item.cpp
        src/gui/qt_input_service_proxy.cpp
        src/app/settings/settings_service.cpp
        src/gui/EventEmitter.h
        src/gui/qt_input_service_proxy.hpp
        src/gui/qt_emulation_service_proxy.cpp
        src/gui/qt_emulation_service_proxy.hpp
        src/image_cache.cpp
        src/app/emulation/emulation_service.cpp
        src/app/emulation/emulator_instance.cpp
        src/app/settings/emulation_setting.hpp
        src/gui/models/game_activity_list_model.cpp
        src/gui/models/game_activity_list_model.hpp
)

target_include_directories(firelight_lib PUBLIC ${SDL2_INCLUDE_DIRS} ${FFMPEG_INCLUDE_DIRS} include ${CMAKE_SOURCE_DIR}/src/app)
target_link_directories(firelight_lib PUBLIC ${FFMPEG_LIBRARY_DIRS} ${CMAKE_SOURCE_DIR}/lib/discord_social_sdk/lib/debug)

if (WIN32)
    target_link_libraries(firelight_lib PUBLIC mingw32)
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
        Qt6::Sql
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
        library
        patching
        rcheevos
        discord
)

# qt_import_plugins(firelight_lib
#         INCLUDE QFFmpegMediaPlugin)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/app_resources.rc")

# main code
qt6_add_executable(firelight
        resources.qrc
        app_resources.rc
        src/main.cpp
        src/gui/models/emulation_settings_model.cpp
        src/gui/models/emulation_settings_model.hpp
)


if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set_target_properties(firelight PROPERTIES
            MACOSX_BUNDLE ON
    )
endif ()

configure_file(${CMAKE_SOURCE_DIR}/qtquickcontrols2.conf ${CMAKE_BINARY_DIR}/qtquickcontrols2.conf COPYONLY)

qt6_add_qml_module(firelight
        URI QMLFirelight
        VERSION 1.0
        QML_FILES
        ${QML_FILES}
)

#target_include_directories(firelight PRIVATE ${SDL2_INCLUDE_DIRS})
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

file(COPY ${CMAKE_SOURCE_DIR}/_cores DESTINATION ${CMAKE_BINARY_DIR}/system)
file(COPY ${CMAKE_SOURCE_DIR}/content.db DESTINATION ${CMAKE_BINARY_DIR}/system)

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
        tests/app/patching/pm_star_rod_mod_patch_test.cpp
        tests/app/patching/ips_patch_test.cpp
        tests/app/patching/ups_patch_test.cpp
        tests/app/patching/bps_patch_test.cpp
        tests/app/patching/util_test.cpp
        tests/mocks/mock_library_database.hpp
        tests/app/db/sqlite_userdata_database_test.cpp
        tests/app/saves/save_manager_test.cpp
        tests/app/library/sqlite_library_database_test.cpp
        tests/app/library/playlist_test.cpp
        tests/app/library/library_entry_test.cpp
        tests/app/db/daos/savefile_metadata_test.cpp
        tests/app/db/daos/play_session_test.cpp
        tests/app/db/sqlite_content_database_test.cpp
        tests/app/rcheevos/ra_cache_test.cpp
        tests/app/rcheevos/rcheevos_offline_client_test.cpp
        tests/app/settings/sqlite_settings_repository_test.cpp
        tests/app/settings/settings_service_test.cpp

        tests/main.cpp
        tests/app/library/sqlite_user_library_test.cpp
        tests/app/platforms/platform_service_test.cpp
        tests/app/input/input_service_impl_test.cpp
        tests/app/input/test_gamepad.cpp
        tests/app/input/test_gamepad.hpp
        tests/app/emulation/emulation_service_test.cpp
        tests/app/emulation/emulator_instance_test.cpp
        tests/mocks/mock_library.hpp
)

file(GLOB QML_TEST_FILES
        qml/ColorPalette.qml
        qml/Constants.qml
        qml/AppStyle.qml
        #        qml/components/IconButton.qml
        qml/components/FLFocusHighlight.qml
        qml_tests/components/tst_FLFocusHighlight.qml
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
add_definitions(-DQUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_test(NAME QMLTests COMMAND fl_qml_test)

#target_compile_options(fl_test PUBLIC "-fprofile-instr-generate" "-fcoverage-mapping")
#target_link_options(fl_test PUBLIC "-fprofile-instr-generate" "-fcoverage-mapping")

add_dependencies(fl_test copy_resources)

target_include_directories(fl_test PRIVATE ${GTEST_INCLUDE_DIRS} include)
target_link_libraries(fl_test PUBLIC firelight_lib ${GTEST_BOTH_LIBRARIES})
gtest_discover_tests(fl_test)
#ctest_coverage_collect_gcov()
#target_compile_options(firelight PRIVATE -Werror -ggdb3 -O0)
#
#add_test(NAME firelight_test COMMAND firelight)

#file(COPY LICENSE.md README.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#install(FILES LICENSE.md README.txt DESTINATION ${CMAKE_INSTALL_BINDIR})

set(CPACK_PACKAGE_NAME "Firelight")
set(CPACK_PACKAGE_VENDOR "BiscuitCakes")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 9)
set(CPACK_PACKAGE_VERSION_PATCH 5)
set(CPACK_PACKAGE_VERSION "0.11.0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Firelight")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_EXECUTABLES "firelight;Firelight")

set(CPACK_NSIS_DISPLAY_NAME "Firelight")
set(CPACK_NSIS_PACKAGE_NAME "Firelight")
set(CPACK_NSIS_UNINSTALL_NAME "Firelight Uninstaller")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/assets/img/icon.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/assets/img/icon.ico")

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
        DESTINATION ${CMAKE_INSTALL_BINDIR}/system/
)

install(
        DIRECTORY
        ${CMAKE_SOURCE_DIR}/_cores
        DESTINATION ${CMAKE_INSTALL_BINDIR}/system/
)

set(CPACK_INSTALL_SCRIPT "${CMAKE_SOURCE_DIR}/cpack_portable.cmake")

include(CPack)
