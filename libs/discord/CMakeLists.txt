file(GLOB DISCORD_SOURCES
        src/*.cpp
        src/*.h
)

add_library(discord ${DISCORD_SOURCES})
target_include_directories(discord PUBLIC ${CMAKE_SOURCE_DIR}/include/discord)
target_link_directories(discord PUBLIC ${CMAKE_SOURCE_DIR}/lib/x86_64)
target_link_libraries(discord PUBLIC discord_game_sdk)
#target_link_libraries(discord PUBLIC ${CMAKE_SOURCE_DIR}/lib/x86_64/discord_game_sdk.dll)