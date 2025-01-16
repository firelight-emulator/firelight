file(GLOB DISCORD_SOURCES
        src/*.cpp
        src/*.h
)

add_library(discord ${DISCORD_SOURCES})