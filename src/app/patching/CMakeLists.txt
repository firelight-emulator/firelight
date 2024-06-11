find_package(ZLIB REQUIRED)
find_package(spdlog REQUIRED)

set(PATCHING_SOURCES
        ips_patch.cpp
        pm_star_rod_mod_patch.cpp
        yay_0_codec.cpp
        ups_patch.cpp
        bps_patch.cpp)

add_library(patching ${PATCHING_SOURCES})

target_link_libraries(patching PRIVATE spdlog::spdlog ZLIB::ZLIB)