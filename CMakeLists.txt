cmake_minimum_required(VERSION 3.8)
project(MinecraftClone)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/raylib EXCLUDE_FROM_ALL)

add_compile_options(-g -Wall -Wextra -Wno-missing-field-initializers)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h src/*.hpp")

add_executable(MinecraftClone ${SOURCES})

target_link_libraries(MinecraftClone PUBLIC raylib)
