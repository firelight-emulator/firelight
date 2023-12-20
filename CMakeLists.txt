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
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Gui OpenGL QuickControls2 Quick3D REQUIRED)
find_package(GTest)

qt_standard_project_setup(REQUIRES 6.5)

set(CMAKE_CXX_STANDARD 17)

add_library(firelight_lib
        src/app/libretro/core.cpp
        src/app/libretro/coreoption.cpp
        src/app/libretro/coreoption.hpp
        src/app/libretro/game.cpp
        src/app/libretro/game.hpp
        src/app/libretro/video.cpp
        src/app/libretro/video.hpp
        src/app/metrics/gameloop.cpp
        src/app/metrics/gameloop.hpp
        src/app/controller/controller_manager.cpp
        src/app/controller/controller_manager.hpp
        includes/imgui/imconfig.h
        includes/imgui/imgui.cpp
        includes/imgui/imgui.h
        includes/imgui/imgui_demo.cpp
        includes/imgui/imgui_draw.cpp
        includes/imgui/imgui_impl_opengl3.cpp
        includes/imgui/imgui_impl_opengl3.h
        includes/imgui/imgui_impl_opengl3_loader.h
        includes/imgui/imgui_impl_sdl2.cpp
        includes/imgui/imgui_impl_sdl2.h
        includes/imgui/imgui_internal.h
        includes/imgui/imgui_stdlib.cpp
        includes/imgui/imgui_stdlib.h
        includes/imgui/imgui_tables.cpp
        includes/imgui/imgui_widgets.cpp
        includes/imgui/imstb_rectpack.h
        includes/imgui/imstb_textedit.h
        includes/imgui/imstb_truetype.h
        includes/imgui/imgui_impl_opengl3.cpp
        src/lib/gui/style.hpp
        src/lib/graphics/character.hpp
        src/lib/graphics/opengl/shaders.hpp
        src/lib/graphics/opengl/shaders.cpp
        src/lib/graphics/color.hpp
        src/lib/gui/widgets/widget.cpp
        src/lib/gui/widgets/widget.hpp
        src/lib/math/bbox.hpp
        src/lib/math/bbox.cpp
        src/lib/gui/widget_painter.hpp
        src/lib/gui/widget_painter.cpp
        src/lib/graphics/driver.hpp
        src/lib/graphics/opengl/open_gl_driver.cpp
        src/lib/graphics/opengl/open_gl_driver.hpp
        src/lib/gui/widgets/container_widget.cpp
        src/lib/gui/widgets/container_widget.hpp
        src/lib/gui/events/event.hpp
        src/lib/gui/widgets/button.cpp
        src/lib/gui/widgets/button.hpp
        src/lib/gui/widget_factory.cpp
        src/lib/gui/widget_factory.hpp
        src/lib/gui/layout_manager.hpp
        src/lib/gui/vertical_box_layout_manager.cpp
        src/lib/gui/vertical_box_layout_manager.hpp
        src/lib/math/position.hpp
        src/lib/graphics/texture.hpp
        src/lib/gui/screens/screen.hpp
        src/lib/gui/screens/screen_manager.cpp
        src/lib/gui/screens/screen_manager.hpp
        src/lib/gui/screens/screen.cpp
        src/app/screen_thing.cpp
        src/app/screen_thing.hpp
        src/app/db/game_repository.hpp
        src/app/db/game_record.hpp
        src/app/library/entry.hpp
        src/app/db/game_scanner.cpp
        src/app/db/game_scanner.hpp
        src/app/db/in_memory_game_repository.cpp
        src/app/db/in_memory_game_repository.hpp
        src/app/game_screen.cpp
        src/app/game_screen.hpp
        src/lib/graphics/opengl/open_gl_texture.cpp
        src/lib/graphics/opengl/open_gl_texture.hpp
        src/app/library/game_library.cpp
        src/app/library/game_library.hpp
        src/app/saves/save_manager.cpp
        src/app/saves/save_manager.hpp
        src/app/app_constants.hpp
        src/app/patching/pm_star_rod_mod_patch.cpp
        src/app/patching/yay_0_codec.cpp
        src/app/patching/yay_0_codec.hpp
        src/app/patching/pm_star_rod_mod_patch.hpp
        src/app/db/romhack_record.hpp
        src/app/db/romhack_version_record.hpp
        src/lib/graphics/framebuffer.hpp
        src/app/controller/sdl_controller.cpp
        src/app/controller/sdl_controller.hpp
        src/lib/audio/source.hpp
        src/lib/audio/sdl_source.cpp
        src/lib/audio/sdl_source.hpp
        src/app/library/library_manager.cpp
        src/app/library/library_manager.hpp
        src/app/gui/themes.hpp
        src/app/libretro/video_data_receiver.hpp
        src/app/emulation_manager.cpp)

target_include_directories(firelight_lib PRIVATE includes includes/imgui ${SDL2_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS} ${SQLite3_INCLUDE_DIRS})
target_link_libraries(firelight_lib mingw32 SDL2main SDL2 glew32 opengl32 freetype ssl crypto ${SQLite3_LIBRARIES}
        Qt6::Gui Qt6::Quick Qt6::OpenGL Qt6::QuickControls2 Qt6::Quick3D)

# main code
qt_add_executable(firelight
        Main.qml
        src/main.cpp
        src/logo.ico
        src/rom_model.cpp
        src/rom_model.hpp
        src/gui/fl_game.cpp
        src/gui/fl_game.hpp
        src/gui/emulator_view.cpp
        src/gui/emulator_view.hpp
        src/gui/emulator_renderer.cpp
        src/gui/emulator_renderer.hpp
        src/counter.cpp
)

qt_add_qml_module(firelight
        URI QtFirelight
        VERSION 1.0
        QML_FILES Main.qml
)

target_include_directories(firelight PRIVATE includes includes/imgui ${SDL2_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS}
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

