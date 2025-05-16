set(DISCORD_SDK_PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/lib/discord_social_sdk")
set(DISCORD_SDK_INCLUDE_DIR "${DISCORD_SDK_PROJECT_ROOT_DIR}/include")

set(DISCORD_SDK_LIB_DIR_RELEASE "${DISCORD_SDK_PROJECT_ROOT_DIR}/lib/release")
set(DISCORD_SDK_BIN_DIR_RELEASE "${DISCORD_SDK_PROJECT_ROOT_DIR}/bin/release") # For Windows DLL

set(DISCORD_LIB_FILENAME_BASE "discord_partner_sdk")

if (WIN32)
    set(EXPECTED_DISCORD_IMPORT_LIB_PATH "${DISCORD_SDK_LIB_DIR_RELEASE}/${DISCORD_LIB_FILENAME_BASE}.lib")
    set(EXPECTED_DISCORD_SHARED_LIB_PATH "${DISCORD_SDK_BIN_DIR_RELEASE}/${DISCORD_LIB_FILENAME_BASE}.dll")
elseif (APPLE)
    set(EXPECTED_DISCORD_IMPORT_LIB_PATH "${DISCORD_SDK_LIB_DIR_RELEASE}/lib${DISCORD_LIB_FILENAME_BASE}.dylib")
    set(EXPECTED_DISCORD_SHARED_LIB_PATH "${DISCORD_SDK_LIB_DIR_RELEASE}/lib${DISCORD_LIB_FILENAME_BASE}.dylib")
else () # Linux
    set(EXPECTED_DISCORD_IMPORT_LIB_PATH "${DISCORD_SDK_LIB_DIR_RELEASE}/lib${DISCORD_LIB_FILENAME_BASE}.so")
    set(EXPECTED_DISCORD_SHARED_LIB_PATH "${DISCORD_SDK_LIB_DIR_RELEASE}/lib${DISCORD_LIB_FILENAME_BASE}.so")
endif ()

add_library(discord SHARED IMPORTED GLOBAL)

#add_library(discord
#        "${DISCORD_SDK_INCLUDE_DIR}/cdiscord.h"
#        "${DISCORD_SDK_INCLUDE_DIR}/discordpp.h"
#)
#
target_include_directories(discord INTERFACE ${DISCORD_SDK_INCLUDE_DIR})
#target_link_directories(discord PUBLIC ${DISCORD_SDK_LIB_DIR_RELEASE})
#target_link_libraries(discord INTERFACE ${DISCORD_LIB_FILENAME_BASE})

set_property(TARGET discord PROPERTY
        IMPORTED_LOCATION "${EXPECTED_DISCORD_SHARED_LIB_PATH}")
set_property(TARGET discord PROPERTY
        IMPORTED_IMPLIB "${EXPECTED_DISCORD_IMPORT_LIB_PATH}")
set_property(TARGET discord PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${DISCORD_SDK_INCLUDE_DIR}")

message(STATUS "Discord SDK: Target 'discord' configured successfully.")
message(STATUS "  Include directory: ${DISCORD_SDK_INCLUDE_DIR}")
message(STATUS "  Import Library for linking: ${EXPECTED_DISCORD_IMPORT_LIB_PATH}")
message(STATUS "  Runtime Shared Library: ${EXPECTED_DISCORD_SHARED_LIB_PATH}")

#
## Prevent multiple inclusions if this directory were somehow processed multiple times
#if (TARGET Discord::PartnerSDK)
#    message(STATUS "Discord::PartnerSDK target already exists. Skipping re-definition in libs/discord.")
#    return()
#endif ()
#
#message(STATUS "Processing Discord SDK setup in libs/discord/CMakeLists.txt...")
#
## --- Define SDK paths ---
## ${CMAKE_SOURCE_DIR} is the root of your 'firelight' project.
## This assumes your 'lib/discord_social_sdk' directory is at the root of the 'firelight' project.
#set(DISCORD_SDK_PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/lib/discord_social_sdk")
#set(DISCORD_SDK_INCLUDE_DIR "${DISCORD_SDK_PROJECT_ROOT_DIR}/include")
#
#set(DISCORD_SDK_LIB_DIR_RELEASE "${DISCORD_SDK_PROJECT_ROOT_DIR}/lib/release")
#set(DISCORD_SDK_BIN_DIR_RELEASE "${DISCORD_SDK_PROJECT_ROOT_DIR}/bin/release") # For Windows DLL
#
#set(DISCORD_LIB_FILENAME_BASE "discord_partner_sdk")
#
#if (WIN32)
#    set(EXPECTED_DISCORD_IMPORT_LIB_PATH "${DISCORD_SDK_LIB_DIR_RELEASE}/${DISCORD_LIB_FILENAME_BASE}.lib")
#    set(EXPECTED_DISCORD_SHARED_LIB_PATH "${DISCORD_SDK_BIN_DIR_RELEASE}/${DISCORD_LIB_FILENAME_BASE}.dll")
#elseif (APPLE)
#    set(EXPECTED_DISCORD_IMPORT_LIB_PATH "${DISCORD_SDK_LIB_DIR_RELEASE}/lib${DISCORD_LIB_FILENAME_BASE}.dylib")
#    set(EXPECTED_DISCORD_SHARED_LIB_PATH "${DISCORD_SDK_LIB_DIR_RELEASE}/lib${DISCORD_LIB_FILENAME_BASE}.dylib")
#else () # Linux
#    set(EXPECTED_DISCORD_IMPORT_LIB_PATH "${DISCORD_SDK_LIB_DIR_RELEASE}/lib${DISCORD_LIB_FILENAME_BASE}.so")
#    set(EXPECTED_DISCORD_SHARED_LIB_PATH "${DISCORD_SDK_LIB_DIR_RELEASE}/lib${DISCORD_LIB_FILENAME_BASE}.so")
#endif ()
#
## --- Perform crucial existence checks ---
#if (NOT EXISTS "${DISCORD_SDK_INCLUDE_DIR}")
#    message(FATAL_ERROR "Discord SDK Include directory NOT FOUND: ${DISCORD_SDK_INCLUDE_DIR}")
#endif ()
#message(STATUS "Discord SDK Include directory FOUND: ${DISCORD_SDK_INCLUDE_DIR}")
#
#if (NOT EXISTS "${EXPECTED_DISCORD_IMPORT_LIB_PATH}")
#    message(FATAL_ERROR "Discord SDK Import Library NOT FOUND: ${EXPECTED_DISCORD_IMPORT_LIB_PATH}")
#endif ()
#message(STATUS "Discord SDK Import Library FOUND: ${EXPECTED_DISCORD_IMPORT_LIB_PATH}")
#
#if (NOT EXISTS "${EXPECTED_DISCORD_SHARED_LIB_PATH}")
#    message(FATAL_ERROR "Discord SDK Shared Library (DLL/SO/DYLIB) NOT FOUND: ${EXPECTED_DISCORD_SHARED_LIB_PATH}")
#endif ()
#message(STATUS "Discord SDK Shared Library (DLL/SO/DYLIB) FOUND: ${EXPECTED_DISCORD_SHARED_LIB_PATH}")
#
## --- Create an IMPORTED library target ---
## Using GLOBAL makes the target visible across all directories processed by CMake.
#add_library(Discord::PartnerSDK UNKNOWN IMPORTED GLOBAL)
#
#set_target_properties(Discord::PartnerSDK PROPERTIES
#        INTERFACE_INCLUDE_DIRECTORIES "${DISCORD_SDK_INCLUDE_DIR}"
#)
#
#if (WIN32)
#    set_target_properties(Discord::PartnerSDK PROPERTIES
#            IMPORTED_IMPLIB "${EXPECTED_DISCORD_IMPORT_LIB_PATH}"
#    )
#    # For Windows, IMPORTED_LOCATION can also be set to the DLL,
#    # but IMPLIB is primary for linking. The DLL is needed at runtime.
#    set_target_properties(Discord::PartnerSDK PROPERTIES IMPORTED_LOCATION "${EXPECTED_DISCORD_SHARED_LIB_PATH}")
#else () # APPLE or Linux
#    set_target_properties(Discord::PartnerSDK PROPERTIES
#            IMPORTED_LOCATION "${EXPECTED_DISCORD_IMPORT_LIB_PATH}" # .dylib or .so
#    )
#endif ()
#
## Set the variable for the main CMakeLists.txt to use for copying the runtime library.
## CACHE: Makes it globally available in the CMake cache.
## PARENT_SCOPE: Also sets it in the immediate parent scope (main CMakeLists.txt).
#set(DISCORD_SDK_RUNTIME_LIBRARY_FILE "${EXPECTED_DISCORD_SHARED_LIB_PATH}"
#        CACHE FILEPATH "Path to Discord SDK runtime shared library for copying")
#set(DISCORD_SDK_RUNTIME_LIBRARY_FILE "${EXPECTED_DISCORD_SHARED_LIB_PATH}" PARENT_SCOPE)
#
#message(STATUS "Discord SDK target 'Discord::PartnerSDK' created by libs/discord/CMakeLists.txt.")
#message(STATUS "DISCORD_SDK_RUNTIME_LIBRARY_FILE in libs/discord set to: ${EXPECTED_DISCORD_SHARED_LIB_PATH}")