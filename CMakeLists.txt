cmake_minimum_required(VERSION 3.16)
project(sand-sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands for clangd" FORCE)

add_executable(app 
src/main.c
src/sand_sim.c
src/sand_shader.c
)

# Find raylib installed by Homebrew
find_package(raylib REQUIRED)

target_include_directories(app PRIVATE src)
target_link_libraries(app PRIVATE raylib)

# macOS frameworks (often needed with raylib)
if(APPLE)
target_link_libraries(app PRIVATE
"-framework Cocoa"
"-framework IOKit"
"-framework CoreVideo"
"-framework OpenGL"
)
endif()
