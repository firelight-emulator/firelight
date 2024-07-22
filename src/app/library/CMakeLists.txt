find_package(Qt6 6.5 REQUIRED COMPONENTS Concurrent Sql REQUIRED)
find_package(OpenSSL REQUIRED)

set(LIBRARY_SOURCES
        library_scanner.cpp
        sqlite_library_database.cpp)

add_library(library ${LIBRARY_SOURCES})

target_link_libraries(library PUBLIC Qt6::Concurrent Qt6::Sql spdlog::spdlog fmt::fmt ${OPENSSL_LIBRARIES})
