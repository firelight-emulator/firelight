find_package(Qt6 6.7 REQUIRED COMPONENTS Concurrent Sql REQUIRED)

set(LIBRARY_SOURCES
        library_scanner.cpp
        sqlite_library_database.cpp
        library_scanner2.cpp
        library_scanner2.hpp
        rom_file.cpp
        sqlite_user_library.cpp
        sqlite_user_library.hpp
)

add_library(library ${LIBRARY_SOURCES})

find_package(LibArchive REQUIRED)

target_link_libraries(library PUBLIC Qt6::Concurrent Qt6::Sql spdlog::spdlog fmt::fmt LibArchive::LibArchive)
