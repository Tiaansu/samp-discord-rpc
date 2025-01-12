file(GLOB_RECURSE sources
    *.cpp
    *.hpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/deps/discord/include
    ${CMAKE_SOURCE_DIR}/deps/json
)

add_library(${PROJECT_NAME} SHARED ${sources})
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG_MODE>)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/deps/discord/include
    ${CMAKE_SOURCE_DIR}/deps/json
)

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".asi")

find_library(DISCORD_RPC_LIB NAMES discord-rpc.lib PATHS ${CMAKE_SOURCE_DIR}/deps/discord/lib)

if (NOT DISCORD_RPC_LIB)
    message (FATAL_ERROR "discord-rpc.lib was not found.")
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE ${DISCORD_RPC_LIB})