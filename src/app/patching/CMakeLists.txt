set(PATCHING_SOURCES
        ips_patch.cpp
        pm_star_rod_mod_patch.cpp
        yay_0_codec.cpp
        ups_patch.cpp
        bps_patch.cpp
        bps_patch.hpp)

add_library(patching ${PATCHING_SOURCES})