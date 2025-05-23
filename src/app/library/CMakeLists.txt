find_package(Qt6 6.7 REQUIRED COMPONENTS Concurrent Sql REQUIRED)

set(LIBRARY_SOURCES
        gui/library_path_model.cpp
        gui/playlist_item_model.cpp
        library_scanner2.cpp
        library_scanner2.hpp
        rom_file.cpp
        sqlite_user_library.cpp
        sqlite_user_library.hpp
        patch_file.cpp
)

add_library(library ${LIBRARY_SOURCES})

find_package(LibArchive REQUIRED)

target_link_libraries(library PUBLIC Qt6::Concurrent Qt6::Sql spdlog::spdlog fmt::fmt LibArchive::LibArchive)
