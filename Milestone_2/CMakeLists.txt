cmake_minimum_required(VERSION 3.10)
project(OS_Simulator C)

set(CMAKE_C_STANDARD 99)

# Find PkgConfig
find_package(PkgConfig REQUIRED)

# Find GTK4 using PkgConfig
# This will populate GTK4_FOUND, GTK4_INCLUDE_DIRS, GTK4_LIBRARIES, GTK4_CFLAGS, etc.
pkg_check_modules(GTK4 REQUIRED gtk4)

# Verify that pkg_check_modules successfully found GTK4
if (NOT GTK4_FOUND)
    message(FATAL_ERROR "Could not find GTK4 using pkg-config.")
endif()

# (Optional but good practice) Print the found variables for debugging
message(STATUS "GTK4 found:")
message(STATUS "  Include Dirs: ${GTK4_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${GTK4_LIBRARIES}")
message(STATUS "  C Flags: ${GTK4_CFLAGS}") # Includes -I flags and other necessary compiler options

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SRC_FILES
    src/FCFS.c
    src/MLFQ.c
    src/PCB.c
    src/Queue.c
    src/RoundRobin.c
    src/index.c
    src/instruction.c
    src/memory.c
    src/memory_manager.c
    src/mutex.c
    src/parser.c
    src/process.c
    src/views/console_view.c
    src/views/gui_globals.c
)

set(GUI_MAIN src/gui_main.c)

# Add the executable target
add_executable(gui_simulator ${GUI_MAIN} ${SRC_FILES})

# Add include directories for your project's headers and GTK4's headers
# Use target_include_directories for better practice
target_include_directories(gui_simulator PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    # Consider if src and src/views really need to be in the include path for headers
    # ${CMAKE_SOURCE_DIR}/src
    # ${CMAKE_SOURCE_DIR}/src/views
    ${GTK4_INCLUDE_DIRS} # <-- Use the variable from pkg_check_modules: GTK4_INCLUDE_DIRS
)

# Add compiler options provided by pkg-config (like -mfpmath=sse, etc.)
target_compile_options(gui_simulator PRIVATE ${GTK4_CFLAGS})

# Link against GTK4 libraries and Threads
target_link_libraries(gui_simulator PRIVATE
    ${GTK4_LIBRARIES} # <-- Use the variable from pkg_check_modules: GTK4_LIBRARIES
    Threads::Threads
)