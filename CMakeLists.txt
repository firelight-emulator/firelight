cmake_minimum_required(VERSION 3.26.4)
project(firelight)

#find_package(SDL REQUIRED)
#find_package(GLEW REQUIRED)
#find_package(OpenGL REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(includes)
include_directories(includes/imgui)
include_directories(includes/SDL2)

link_directories(lib)

# Include Google Test as an external project
#include(ExternalProject)
#ExternalProject_Add(googletest
#        GIT_REPOSITORY https://github.com/google/googletest.git
#        GIT_TAG master
#        PREFIX ${CMAKE_BINARY_DIR}/gtest
#        INSTALL_COMMAND ""
#)
#
## Set GTest source and binary directories
#ExternalProject_Get_Property(googletest source_dir binary_dir)
#
## Add Google Test headers to your project's include directories
#include_directories(${source_dir}/googletest/include)

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
        src/lib/gui/signal.hpp
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
        src/app/platforms/platform_configuration.hpp
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
        src/lib/audio/sdl_source.hpp)

target_link_libraries(firelight_lib mingw32 SDL2main SDL2 glew32 opengl32 freetype ssl crypto gtest_main)

# main code
add_executable(firelight
        src/main.cpp
        src/logo.ico)

target_link_libraries(firelight PRIVATE firelight_lib)
target_compile_options(firelight PRIVATE -Werror -ggdb3 -O0)

add_executable(fl_test
        tests/app/patching/pm_star_rod_mod_patch_test.cpp
)

target_link_libraries(fl_test PRIVATE firelight_lib gtest gtest_main)
#target_compile_options(firelight PRIVATE -Werror -ggdb3 -O0)
#
#add_test(NAME firelight_test COMMAND firelight)
#
