if (CPACK_GENERATOR STREQUAL "ZIP")
    file(WRITE "${CPACK_TEMPORARY_INSTALL_DIRECTORY}/bin/portable.txt" "This is a portable build.")
endif ()