cmake_minimum_required(VERSION 3.22.1)
project(firelight
        VERSION 0.6.0
        DESCRIPTION "An emulation frontend that just works"
        #    HOMEPAGE_URL ""
        LANGUAGES CXX
)

#include(${CMAKE_SOURCE_DIR}/cmake/rcheevos.cmake)
#include(${CMAKE_SOURCE_DIR}/cmake/clang-checks.cmake)
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(VCPKG_DISABLE_COMPILER_TRACKING ON)

set(CMAKE_BUILD_RPATH ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})

list(APPEND CMAKE_PREFIX_PATH "/Users/alexcharles/Qt/6.7.2/macos" "/opt/homebrew" "C:/Qt/6.8.1/mingw_64")

list(APPEND CMAKE_INCLUDE_PATH "/opt/homebrew/include")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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

find_package(SDL2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Qt6 6.8 REQUIRED COMPONENTS Quick Gui OpenGL QuickControls2 Quick3D Sql Charts QuickTest Multimedia REQUIRED)
find_package(cpr REQUIRED)
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(FFmpeg REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE SWRESAMPLE)
find_package(LibArchive REQUIRED)
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavcodec libavformat libswresample)

qt6_standard_project_setup(REQUIRES 6.7)

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

file(GLOB QML_FILES
        qml/*.qml
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
set_source_files_properties(qml/NewRouter.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

add_library(firelight_lib
        src/app/libretro/core.cpp
        src/app/libretro/game.cpp
        src/app/db/sqlite_content_database.cpp
        src/app/input/sdl_controller.cpp
        src/app/input/controller_manager.cpp
        src/app/input/sdl_event_loop.cpp
        src/app/manager_accessor.cpp
        src/app/audio/audio_manager.cpp
        src/app/saves/save_manager.cpp
        src/app/saves/savefile.cpp
        src/app/db/sqlite_userdata_database.cpp
        src/app/input/controller_list_model.cpp
        src/app/library/library_item_model.cpp
        src/app/library/library_sort_filter_model.cpp
        src/app/library/playlist_item_model.cpp
        src/app/saves/savefile_list_model.cpp
        src/app/mods/mod_manager.cpp
        src/app/rcheevos/ra_client.cpp
        src/gui/eventhandlers/window_resize_handler.cpp
        src/app/library/library_path_model.cpp
        src/app/rcheevos/achievement_list_model.cpp
        src/app/rcheevos/achievement_list_sort_filter_model.cpp
        src/gui/router.cpp
        src/app/input/profiles/controller_profile.cpp
        src/app/input/profiles/input_mapping.cpp
        src/gui/platform_list_model.cpp
        src/app/libretro/core_configuration.cpp
        src/app/libretro/core_configuration.hpp
        src/app/emulator_config_manager.cpp
        src/app/platform_metadata.hpp
        src/gui/models/shop/shop_item_model.cpp
        src/gui/models/shop/shop_item_model.hpp
        src/gui/models/suspend_point_list_model.cpp
        src/gui/models/suspend_point_list_model.hpp
        src/gui/game_image_provider.cpp
        src/gui/game_image_provider.hpp
        src/app/audio/SfxPlayer.cpp
        src/app/audio/SfxPlayer.hpp
        src/app/rcheevos/regular_http_client.cpp
        src/app/rcheevos/regular_http_client.hpp
        src/app/rcheevos/offline/earned_achievement.hpp
        src/app/rcheevos/offline/ra_cache.cpp
        src/app/rcheevos/login2_response.hpp
        src/app/rcheevos/patch_response.hpp
        src/app/rcheevos/startsession_response.hpp
        src/app/rcheevos/award_achievement_response.hpp
        src/app/rcheevos/offline/rcheevos_offline_client.hpp
        src/app/rcheevos/offline/rcheevos_offline_client.cpp
        src/gui/eventhandlers/input_method_detection_handler.cpp
        src/gui/eventhandlers/input_method_detection_handler.hpp
        src/app/input/keyboard_input_handler.cpp
        src/gui/models/library/entry_list_model.cpp
        src/app/emulator_item.hpp
        src/app/emulator_item_renderer.hpp
        src/app/emulator_item_renderer.cpp
        src/app/emulator_item.cpp
        src/app/game_loader.cpp
        src/app/game_loader.hpp
        src/app/input/gamepad_status_item.cpp
        src/app/input/gamepad_status_item.hpp
        src/app/input/controller_icons.hpp
        src/app/activity/sqlite_activity_log.cpp
        src/app/activity/sqlite_activity_log.hpp
        src/app/PlatformMetadataItem.cpp
        src/app/PlatformMetadataItem.hpp
        src/app/input/sqlite_controller_repository.cpp
        src/app/input/sdl_controller.hpp
        src/app/input/gamepad.hpp
        src/app/input/input_manager.hpp
        src/app/input/input_manager.cpp
        src/app/input/input_mapping_item.cpp
        src/app/library_entry_item.cpp
        src/app/input/profiles/keyboard_mapping.hpp
        src/app/input/profiles/keyboard_mapping.cpp
)

target_include_directories(firelight_lib PUBLIC ${SDL2_INCLUDE_DIRS} ${FFMPEG_INCLUDE_DIRS} include)
target_link_directories(firelight_lib PUBLIC ${FFMPEG_LIBRARY_DIRS})

if (WIN32)
    target_link_libraries(firelight_lib PUBLIC mingw32)
endif ()

target_link_libraries(firelight_lib PUBLIC
        SDL2::SDL2
        z
        nlohmann_json::nlohmann_json
        Qt6::Gui
        Qt6::Quick
        Qt6::OpenGL
        Qt6::QuickControls2
        Qt6::Quick3D
        Qt6::Sql
        Qt6::Charts
        Qt6::QuickTest
        Qt6::Multimedia
        Qt6::GuiPrivate
        #        PkgConfig::FFMPEG
        ${FFMPEG_LIBRARIES}
        LibArchive::LibArchive
        spdlog::spdlog
        cpr::cpr
        library
        patching
        rcheevos)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/app_resources.rc")

# main code
qt6_add_executable(firelight
        resources.qrc
        app_resources.rc
        src/main.cpp
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
        firelight_lib)

#if (CMAKE_BUILD_TYPE STREQUAL "Release")
#    set_property(TARGET firelight PROPERTY WIN32_EXECUTABLE true)
#endif ()

find_program(MINGW_PATH gcc)
get_filename_component(MINGW_DIR ${MINGW_PATH} DIRECTORY)
set(SHARED_LIB_DIR "${MINGW_DIR}")

#include(GNUInstallDirs)
install(TARGETS firelight
        RUNTIME_DEPENDENCIES
        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" "Qt6"
        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
        DIRECTORIES ${SHARED_LIB_DIR}
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
file(COPY ${CMAKE_SOURCE_DIR}/_img DESTINATION ${CMAKE_BINARY_DIR}/system)
file(COPY ${CMAKE_SOURCE_DIR}/content.db DESTINATION ${CMAKE_BINARY_DIR}/system)
file(COPY ${CMAKE_SOURCE_DIR}/assets/click.wav DESTINATION ${CMAKE_BINARY_DIR}/system/sfx)
file(COPY ${CMAKE_SOURCE_DIR}/assets/openrewind.wav DESTINATION ${CMAKE_BINARY_DIR}/system/sfx)
file(COPY ${CMAKE_SOURCE_DIR}/assets/nope.wav DESTINATION ${CMAKE_BINARY_DIR}/system/sfx)
file(COPY ${CMAKE_SOURCE_DIR}/assets/openquick.wav DESTINATION ${CMAKE_BINARY_DIR}/system/sfx)
file(COPY ${CMAKE_SOURCE_DIR}/assets/closequick.wav DESTINATION ${CMAKE_BINARY_DIR}/system/sfx)
file(COPY ${CMAKE_SOURCE_DIR}/assets/menumove.wav DESTINATION ${CMAKE_BINARY_DIR}/system/sfx)
file(COPY ${CMAKE_SOURCE_DIR}/assets/startgame.wav DESTINATION ${CMAKE_BINARY_DIR}/system/sfx)
file(COPY ${CMAKE_SOURCE_DIR}/assets/defaultnotification.wav DESTINATION ${CMAKE_BINARY_DIR}/system/sfx)
file(COPY ${CMAKE_SOURCE_DIR}/assets/switchon.wav DESTINATION ${CMAKE_BINARY_DIR}/system/sfx)
file(COPY ${CMAKE_SOURCE_DIR}/assets/switchoff.wav DESTINATION ${CMAKE_BINARY_DIR}/system/sfx)

enable_testing()
include(GoogleTest)

add_executable(fl_test
        tests/app/patching/pm_star_rod_mod_patch_test.cpp
        tests/app/patching/ips_patch_test.cpp
        tests/app/patching/ups_patch_test.cpp
        tests/app/patching/bps_patch_test.cpp
        tests/mocks/mock_library_database.hpp
        tests/app/db/sqlite_userdata_database_test.cpp
        tests/app/saves/save_manager_test.cpp
        tests/app/library/sqlite_library_database_test.cpp
        tests/app/library/playlist_test.cpp
        tests/app/library/library_entry_test.cpp
        tests/app/db/daos/savefile_metadata_test.cpp
        tests/app/db/daos/play_session_test.cpp
        tests/app/db/sqlite_content_database_test.cpp
        tests/main.cpp
)

file(GLOB QML_TEST_FILES
        qml/ColorPalette.qml
        qml/Constants.qml
        qml/AppStyle.qml
        qml/common/FirelightButton.qml
        qml/common/DetailsButton.qml
        qml/common/RightClickMenu.qml
        qml/common/RightClickMenuItem.qml
        qml/common/CarouselText.qml
        qml/common/NavigationTabBar.qml
        qml/library/GameGridItemDelegate.qml
        qml/pages/LibraryPage2.qml
        qml_tests/common/tst_DetailsButton.qml
        qml_tests/common/tst_NavigationTabBar.qml
        qml_tests/library/tst_GameGridItemDelegate.qml
        qml_tests/library/tst_LibraryPage2.qml
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

#add_test(NAME QMLTests COMMAND fl_qml_test -platform offscreen)

#target_compile_options(fl_test PUBLIC "-fprofile-instr-generate" "-fcoverage-mapping")
#target_link_options(fl_test PUBLIC "-fprofile-instr-generate" "-fcoverage-mapping")

add_dependencies(fl_test copy_resources)
#

target_include_directories(fl_test PRIVATE ${GTEST_INCLUDE_DIRS} include)
target_link_libraries(fl_test PUBLIC firelight_lib ${GTEST_BOTH_LIBRARIES})
gtest_discover_tests(fl_test)
#ctest_coverage_collect_gcov()
#target_compile_options(firelight PRIVATE -Werror -ggdb3 -O0)
#
#add_test(NAME firelight_test COMMAND firelight)

#file(COPY LICENSE.md README.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#install(FILES LICENSE.md README.txt DESTINATION ${CMAKE_INSTALL_BINDIR})

set(CPACK_PACKAGE_NAME "firelight-emulator")
set(CPACK_PACKAGE_VENDOR "BiscuitCakes")
set(CPACK_PACKAGE_INSTALL_DIRECTORY, "firelight")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "0.6.0")


#set(CPACK_RESOURCE_FILE_LICENSE, "C:/msys64/mingw64/share/cmake/Templates/ALEX.txt")
#set(CPACK_RESOURCE_FILE_README, "${CMAKE_CURRENT_BINARY_DIR}/README.md")
#set(CPACK_RESOURCE_FILE_WELCOME, "Firelight")


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
            TYPE BIN
    )
endif ()

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

install(
        DIRECTORY
        ${CMAKE_SOURCE_DIR}/_img
        DESTINATION ${CMAKE_INSTALL_BINDIR}/system/
)

install(
        DIRECTORY
        ${CMAKE_SOURCE_DIR}/_mods
        DESTINATION ${CMAKE_INSTALL_BINDIR}/system/
)

install(
        FILES
        ${CMAKE_SOURCE_DIR}/assets/openrewind.wav
        ${CMAKE_SOURCE_DIR}/assets/click.wav
        ${CMAKE_SOURCE_DIR}/assets/nope.wav
        ${CMAKE_SOURCE_DIR}/assets/openquick.wav
        ${CMAKE_SOURCE_DIR}/assets/closequick.wav
        ${CMAKE_SOURCE_DIR}/assets/menumove.wav
        ${CMAKE_SOURCE_DIR}/assets/startgame.wav
        ${CMAKE_SOURCE_DIR}/assets/defaultnotification.wav
        ${CMAKE_SOURCE_DIR}/assets/switchon.wav
        ${CMAKE_SOURCE_DIR}/assets/switchoff.wav
        DESTINATION ${CMAKE_INSTALL_BINDIR}/system/sfx/
)


#install(CODE [[
#  file(GET_RUNTIME_DEPENDENCIES
#    RESOLVED_DEPENDENCIES_VAR RESOLVED
#    EXECUTABLES firelight.app/Contents/MacOS/firelight
#    )
#  message(${RESOLVED})
#  ]])

#install(CODE [[
#    include(BundleUtilities)
#    message ("=======VERIFYING=======")
#    fixup_bundle("/Users/alexcharles/git/firelight/build/debug/firelight-other.app" "" "")
#    message ("=======DONE VERIFYING=======")
#]])


include(CPack)