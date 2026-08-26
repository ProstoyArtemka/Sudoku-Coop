cmake_minimum_required(VERSION 3.16)
project(Sudoku-Coop)

set(CMAKE_C_STANDARD 99)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "5.5"
    GIT_PROGRESS TRUE
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE SOURCES "src/*.c")

add_executable("${PROJECT_NAME}" WIN32 ${SOURCES})

target_include_directories("${PROJECT_NAME}" PRIVATE include libs)

target_link_libraries("${PROJECT_NAME}" raylib ws2_32)

set_target_properties("${PROJECT_NAME}" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)