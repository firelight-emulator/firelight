cmake_minimum_required(VERSION 3.26.4)
project(firelight)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message("This is a Windows platform.")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message("This is a Linux platform.")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message("This is a macOS platform.")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Android")
    message("This is a Android platform.")
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    message("This is a FreeBSD platform.")
else ()
    message("This is an unknown platform. :(")
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
        src/app/controller/fps_multiplier.cpp
        src/app/controller/fps_multiplier.hpp
        src/app/db/library_database.hpp
        src/app/db/sqlite_library_database.cpp
        src/app/db/sqlite_library_database.hpp
        src/app/db/daos/rom.hpp
        src/app/db/daos/game.hpp
        src/app/db/daos/lib_entry.hpp
        src/app/db/daos/platform.hpp
        src/app/libretro/retropad.hpp
        src/gui/controller.hpp
        src/gui/controller.cpp
        src/app/libretro/retropad_provider.hpp
        src/gui/controller_manager.cpp
        src/gui/controller_manager.hpp
        src/gui/sdl_event_loop.cpp
        src/gui/sdl_event_loop.hpp
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
)

target_include_directories(firelight_lib PRIVATE includes includes/imgui ${SDL2_INCLUDE_DIRS}
        ${SQLite3_INCLUDE_DIRS})
target_link_libraries(firelight_lib mingw32 SDL2main SDL2 opengl32 ssl crypto ${SQLite3_LIBRARIES}
        Qt6::Gui Qt6::Quick Qt6::OpenGL Qt6::QuickControls2 Qt6::Quick3D Qt6::Sql spdlog::spdlog)

# main code
qt_add_executable(firelight
        Main.qml
        Notification.qml
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

qt_add_qml_module(firelight
        URI QtFirelight
        VERSION 1.0
        QML_FILES
        Main.qml
        Notification.qml
)

target_include_directories(firelight PRIVATE includes includes/imgui ${SDL2_INCLUDE_DIRS}
        ${SQLite3_INCLUDE_DIRS})
target_link_libraries(firelight PRIVATE firelight_lib)


include(GNUInstallDirs)
install(TARGETS firelight
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(fl_test
        tests/app/patching/pm_star_rod_mod_patch_test.cpp
)

target_include_directories(fl_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(fl_test PRIVATE firelight_lib ${GTEST_BOTH_LIBRARIES})
#target_compile_options(firelight PRIVATE -Werror -ggdb3 -O0)
#
#add_test(NAME firelight_test COMMAND firelight)

