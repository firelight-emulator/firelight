cmake_minimum_required(VERSION 3.22.1)
project(firelight)

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
find_package(OpenGL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Gui OpenGL QuickControls2 Quick3D Sql REQUIRED)
find_package(GTest)

qt_standard_project_setup(REQUIRES 6.5)

set(CMAKE_CXX_STANDARD 17)

add_library(firelight_lib
        #        ${DISCORD_SOURCES}
        src/app/libretro/core.cpp
        src/app/libretro/coreoption.cpp
        src/app/libretro/coreoption.hpp
        src/app/libretro/game.cpp
        src/app/libretro/game.hpp
        src/app/saves/save_manager.cpp
        src/app/saves/save_manager.hpp
        src/app/patching/pm_star_rod_mod_patch.cpp
        src/app/patching/yay_0_codec.cpp
        src/app/patching/yay_0_codec.hpp
        src/app/patching/pm_star_rod_mod_patch.hpp
        src/app/libretro/video_data_receiver.hpp
        src/app/emulation_manager.cpp
        src/app/db/content_database.hpp
        src/app/db/sqlite_content_database.cpp
        src/app/db/sqlite_content_database.hpp
        src/app/fps_multiplier.cpp
        src/app/fps_multiplier.hpp
        src/app/db/library_database.hpp
        src/app/db/sqlite_library_database.cpp
        src/app/db/sqlite_library_database.hpp
        src/app/db/daos/rom.hpp
        src/app/db/daos/game.hpp
        src/app/db/daos/lib_entry.hpp
        src/app/db/daos/platform.hpp
        src/app/libretro/retropad.hpp
        src/app/input/controller.hpp
        src/app/input/controller.cpp
        src/app/libretro/retropad_provider.hpp
        src/app/input/controller_manager.cpp
        src/app/input/controller_manager.hpp
        src/app/input/sdl_event_loop.cpp
        src/app/input/sdl_event_loop.hpp
        src/app/manager_accessor.hpp
        src/app/manager_accessor.cpp
        src/app/libretro/audio_data_receiver.hpp
        src/app/audio_manager.cpp
        src/app/audio_manager.hpp
        src/gui/save_manager.cpp
        src/gui/save_manager.hpp
        src/app/saves/save_data.hpp
        src/app/saves/save_data.cpp
        src/app/game_loader.cpp
        src/app/game_loader.hpp
        src/app/patching/ips_patch.cpp
        src/app/patching/ips_patch.hpp
        src/app/patching/rom_patch.hpp
        src/app/emulator_renderer.cpp
        src/app/emulator_renderer.hpp
        src/gui/window_resize_handler.cpp
        src/gui/window_resize_handler.hpp
        src/app/input/keyboard_controller.cpp
        src/app/input/keyboard_controller.hpp
        src/app/db/userdata_database.hpp
        src/app/db/sqlite_userdata_database.cpp
        src/app/db/sqlite_userdata_database.hpp
        src/app/input/controller_list_model.cpp
        src/app/input/controller_list_model.hpp
)

target_include_directories(firelight_lib PRIVATE includes ${SDL2_INCLUDE_DIRS}
        ${SQLite3_INCLUDE_DIRS})
target_link_libraries(firelight_lib mingw32 SDL2 opengl32 ssl crypto ${SQLite3_LIBRARIES}
        Qt6::Gui Qt6::Quick Qt6::OpenGL Qt6::QuickControls2 Qt6::Quick3D Qt6::Sql spdlog::spdlog)

file(GLOB QML_FILES
        qml/*.qml
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
qt_add_executable(firelight
        resources.qrc
        src/main.cpp
        src/logo.ico
        src/gui/QLibraryManager.cpp
        src/gui/QLibraryManager.hpp
        src/gui/QLibraryViewModel.cpp
        src/gui/QLibraryViewModel.hpp
        src/app/db/daos/romhack.hpp
        src/app/db/daos/romhack_release.hpp
)


configure_file(${CMAKE_SOURCE_DIR}/qtquickcontrols2.conf ${CMAKE_BINARY_DIR}/qtquickcontrols2.conf COPYONLY)

qt_add_qml_module(firelight
        URI QMLFirelight
        VERSION 1.0
        QML_FILES
        ${QML_FILES}
)

target_include_directories(firelight PRIVATE includes ${SDL2_INCLUDE_DIRS}
        ${SQLite3_INCLUDE_DIRS})
target_link_libraries(firelight PRIVATE firelight_lib)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET firelight PROPERTY WIN32_EXECUTABLE true)
endif ()

set_source_files_properties(qml/FirelightStyle/import/Constants.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(firelight_style
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

#file(GET_RUNTIME_DEPENDENCIES
#        RESOLVED_DEPENDENCIES_VAR _r_deps
#        UNRESOLVED_DEPENDENCIES_VAR _u_deps
#        EXECUTABLES firelight
#        PRE_BUILD)
#
#message("Resolved Dependencies: ${_r_deps}")
#message("Unresolved Dependencies: ${_u_deps}")

add_executable(fl_test
        tests/app/patching/pm_star_rod_mod_patch_test.cpp
        tests/app/patching/ips_patch_test.cpp
)

add_dependencies(fl_test copy_resources)

target_include_directories(fl_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(fl_test PRIVATE firelight_lib ${GTEST_BOTH_LIBRARIES})
#target_compile_options(firelight PRIVATE -Werror -ggdb3 -O0)
#
#add_test(NAME firelight_test COMMAND firelight)
set(CPACK_PACKAGE_NAME "Firelight")
set(CPACK_PACKAGE_VENDOR "BiscuitCakes")

include(CPack)
qt_generate_deploy_qml_app_script(
        TARGET firelight
        OUTPUT_SCRIPT deploy_script
)

install(
        SCRIPT ${deploy_script}
)

set(SHARED_LIB_DIR "C:/msys64/mingw64/bin")
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
        ${SHARED_LIB_DIR}/libsqlite3-0.dll
        ${SHARED_LIB_DIR}/libssl-3-x64.dll
        ${SHARED_LIB_DIR}/libstdc++-6.dll
        ${SHARED_LIB_DIR}/libwinpthread-1.dll
        ${SHARED_LIB_DIR}/SDL2.dll
        TYPE BIN
)
install(
        DIRECTORY
        ${CMAKE_SOURCE_DIR}/_cores
        DESTINATION ${CMAKE_INSTALL_BINDIR}/system/
)