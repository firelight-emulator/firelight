cmake_minimum_required(VERSION 3.22.1)
project(firelight
        DESCRIPTION "An emulation frontend that just works"
        #    HOMEPAGE_URL ""
        LANGUAGES CXX
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message("This is a Windows platform.")
    #elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    #    message("This is a Linux platform.")
    #elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    #    message("This is a macOS platform.")
    #elseif (CMAKE_SYSTEM_NAME STREQUAL "Android")
    #    message("This is a Android platform.")
    #elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    #    message("This is a FreeBSD platform.")
else ()
    message("This CMake file is not set up to work on anything but Windows at the moment. Feel free to tinker!")
    return()
endif ()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif ()

find_package(SDL2 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(spdlog REQUIRED)
find_package(Qt6 6.7 REQUIRED COMPONENTS Quick Gui OpenGL QuickControls2 Quick3D Sql REQUIRED)
find_package(cpr REQUIRED)
find_package(GTest)

#include(FetchContent)
#FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
#        GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8)
#FetchContent_MakeAvailable(cpr)

#find_package(PkgConfig REQUIRED)
#pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
#pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
#pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
#pkg_check_modules(SWSCALE REQUIRED IMPORTED_TARGET libswscale)

qt6_standard_project_setup(REQUIRES 6.7)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(src/app)
add_subdirectory(libs/rcheevos)

add_library(firelight_lib
        #        ${DISCORD_SOURCES}
        src/app/libretro/core.cpp
        src/app/libretro/coreoption.cpp
        src/app/libretro/game.cpp
        src/app/emulation_manager.cpp
        src/app/db/sqlite_content_database.cpp
        src/app/fps_multiplier.cpp
        src/app/input/controller.cpp
        src/app/input/controller_manager.cpp
        src/app/input/sdl_event_loop.cpp
        src/app/manager_accessor.cpp
        src/app/audio_manager.cpp
        src/app/saves/save_manager.cpp
        src/app/saves/savefile.cpp
        src/app/emulator_renderer.cpp
        src/app/input/keyboard_controller.cpp
        src/app/db/sqlite_userdata_database.cpp
        src/gui/controller_list_model.cpp
        src/gui/library_item_model.cpp
        src/gui/library_sort_filter_model.cpp
        src/gui/playlist_item_model.cpp
        src/gui/savefile_list_model.cpp
        src/gui/mod_item_model.cpp
        src/app/mods/mod_manager.cpp
        src/app/achieve/ra_client.cpp
        src/gui/window_resize_handler.cpp
)

target_include_directories(firelight_lib PRIVATE ${SDL2_INCLUDE_DIRS} include)
target_link_libraries(firelight_lib
        mingw32
        SDL2
        ssl
        crypto
        z
        Qt6::Gui
        Qt6::Quick
        Qt6::OpenGL
        Qt6::QuickControls2
        Qt6::Quick3D
        Qt6::Sql
        spdlog::spdlog
        cpr::cpr
        #        PkgConfig::AVCODEC
        #        PkgConfig::AVFORMAT
        #        PkgConfig::AVUTIL
        #        PkgConfig::SWSCALE
        #        library
        #        cpr::cpr
        library
        patching
        rcheevos)

file(GLOB QML_FILES
        qml/*.qml
        qml/common/*.qml
        qml/mainmenu/*.qml
        qml/controllers/*.qml
        qml/settings/*.qml
        qml/library/*.qml
        qml/discover/*.qml
)

foreach (QML_FILE ${QML_FILES})
    get_filename_component(QML_FILE_NAME ${QML_FILE} NAME)
    set_property(SOURCE ${QML_FILE} PROPERTY QT_RESOURCE_ALIAS "${QML_FILE_NAME}")
endforeach ()

file(GLOB_RECURSE QML_STYLE_FILES
        qml/FirelightStyle/**/*.qml
)

foreach (QML_FILE ${QML_STYLE_FILES})
    get_filename_component(QML_FILE_NAME ${QML_FILE} NAME)
    set_property(SOURCE ${QML_FILE} PROPERTY QT_RESOURCE_ALIAS "${QML_FILE_NAME}")
endforeach ()

# main code
qt6_add_executable(firelight
        resources.qrc
        src/main.cpp
        src/logo.ico
        include/firelight/db/patch.hpp
)

configure_file(${CMAKE_SOURCE_DIR}/qtquickcontrols2.conf ${CMAKE_BINARY_DIR}/qtquickcontrols2.conf COPYONLY)

qt6_add_qml_module(firelight
        URI QMLFirelight
        VERSION 1.0
        QML_FILES
        ${QML_FILES}
)

target_include_directories(firelight PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(firelight PRIVATE firelight_lib)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET firelight PROPERTY WIN32_EXECUTABLE true)
endif ()

set_source_files_properties(qml/FirelightStyle/import/Constants.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/FirelightStyle/import/GeneralSettings.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt6_add_qml_module(firelight_style
        URI FirelightStyle
        VERSION 1.0
        QML_FILES
        ${QML_STYLE_FILES}
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/FirelightStyle"
)


include(GNUInstallDirs)
install(TARGETS firelight
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
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

#target_compile_options(fl_test PUBLIC "-fprofile-instr-generate" "-fcoverage-mapping")
#target_link_options(fl_test PUBLIC "-fprofile-instr-generate" "-fcoverage-mapping")

add_dependencies(fl_test copy_resources)

target_include_directories(fl_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(fl_test PRIVATE firelight_lib ${GTEST_BOTH_LIBRARIES} gmock)
#target_compile_options(firelight PRIVATE -Werror -ggdb3 -O0)
#
#add_test(NAME firelight_test COMMAND firelight)

#file(COPY LICENSE.md README.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#install(FILES LICENSE.md README.txt DESTINATION ${CMAKE_INSTALL_BINDIR})

set(CPACK_PACKAGE_NAME "Firelight")
set(CPACK_PACKAGE_VENDOR "BiscuitCakes")
set(CPACK_PACKAGE_INSTALL_DIRECTORY, "Firelight")
#set(CPACK_RESOURCE_FILE_LICENSE, "C:/msys64/mingw64/share/cmake/Templates/ALEX.txt")
#set(CPACK_RESOURCE_FILE_README, "${CMAKE_CURRENT_BINARY_DIR}/README.md")
#set(CPACK_RESOURCE_FILE_WELCOME, "Firelight")

include(CPack)
qt6_generate_deploy_qml_app_script(
        TARGET firelight
        OUTPUT_SCRIPT deploy_script
)


install(
        SCRIPT ${deploy_script}
)

find_program(MINGW_PATH gcc)
get_filename_component(MINGW_DIR ${MINGW_PATH} DIRECTORY)
set(SHARED_LIB_DIR "${MINGW_DIR}")
message("Shared Library Directory: ${SHARED_LIB_DIR}")

install(
        PROGRAMS
        ${CMAKE_BINARY_DIR}/firelight_style.dll
        ${SHARED_LIB_DIR}/libb2-1.dll
        ${SHARED_LIB_DIR}/libbrotlicommon.dll
        ${SHARED_LIB_DIR}/libbrotlidec.dll
        ${SHARED_LIB_DIR}/libbz2-1.dll
        ${SHARED_LIB_DIR}/libcrypto-3-x64.dll
        ${SHARED_LIB_DIR}/libdouble-conversion.dll
        ${SHARED_LIB_DIR}/libfmt.dll
        ${SHARED_LIB_DIR}/libfreetype-6.dll
        ${SHARED_LIB_DIR}/libgcc_s_seh-1.dll
        ${SHARED_LIB_DIR}/libglib-2.0-0.dll
        ${SHARED_LIB_DIR}/libgraphite2.dll
        ${SHARED_LIB_DIR}/libharfbuzz-0.dll
        ${SHARED_LIB_DIR}/libicudt74.dll
        ${SHARED_LIB_DIR}/libicuin74.dll
        ${SHARED_LIB_DIR}/libicuuc74.dll
        ${SHARED_LIB_DIR}/libmd4c.dll
        ${SHARED_LIB_DIR}/libpcre2-8-0.dll
        ${SHARED_LIB_DIR}/libpcre2-16-0.dll
        ${SHARED_LIB_DIR}/libpng16-16.dll
        ${SHARED_LIB_DIR}/libspdlog.dll
        ${SHARED_LIB_DIR}/libssl-3-x64.dll
        ${SHARED_LIB_DIR}/libstdc++-6.dll
        ${SHARED_LIB_DIR}/libwinpthread-1.dll
        ${SHARED_LIB_DIR}/libzstd.dll
        ${SHARED_LIB_DIR}/zlib1.dll
        ${SHARED_LIB_DIR}/libintl-8.dll
        ${SHARED_LIB_DIR}/libiconv-2.dll
        ${SHARED_LIB_DIR}/SDL2.dll
        TYPE BIN
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