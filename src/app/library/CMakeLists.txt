find_package(Qt6 6.7 REQUIRED COMPONENTS Concurrent Sql REQUIRED)

set(LIBRARY_SOURCES
        library_scanner.cpp
        sqlite_library_database.cpp
        library_scanner2.cpp
        library_scanner2.hpp)

add_library(library ${LIBRARY_SOURCES})

target_link_libraries(library PUBLIC Qt6::Concurrent Qt6::Sql spdlog::spdlog fmt::fmt)
