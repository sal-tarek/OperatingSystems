cmake_minimum_required(VERSION 3.10)
project(OS_Simulator C)

set(CMAKE_C_STANDARD 99)

# Find PkgConfig and GTK4
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

# Print GTK4 config for debug
message(STATUS "GTK4 Include Dirs: ${GTK4_INCLUDE_DIRS}")
message(STATUS "GTK4 Libraries: ${GTK4_LIBRARIES}")
message(STATUS "GTK4 C Flags: ${GTK4_CFLAGS}")

# Find threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Automatically gather all .c source files in src/
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.c")

# Define executable target
add_executable(gui_simulator ${SRC_FILES})

# Set include directories
target_include_directories(gui_simulator PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${GTK4_INCLUDE_DIRS}
)

# Set compiler options and link libraries
target_compile_options(gui_simulator PRIVATE ${GTK4_CFLAGS})
target_link_libraries(gui_simulator PRIVATE ${GTK4_LIBRARIES} Threads::Threads)