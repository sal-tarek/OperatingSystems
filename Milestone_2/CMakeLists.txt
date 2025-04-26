cmake_minimum_required(VERSION 3.10)
project(OS_Simulator C)

# Set C standard
set(CMAKE_C_STANDARD 99)

# Find GTK4 for the GUI executable
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

# Print GTK4 configuration for debugging
message("GTK4_INCLUDE_DIRS: ${GTK4_INCLUDE_DIRS}")
message("GTK4_LIBRARIES: ${GTK4_LIBRARIES}")
message("GTK4_LIBRARY_DIRS: ${GTK4_LIBRARY_DIRS}")
message("GTK4_CFLAGS_OTHER: ${GTK4_CFLAGS_OTHER}")

# Include directories for both simulator and GUI
include_directories(
    include
    src
    src/views
    ${GTK4_INCLUDE_DIRS}
)

# Link directories for GTK4
link_directories(${GTK4_LIBRARY_DIRS})

# Add GTK4-specific compiler flags
add_definitions(${GTK4_CFLAGS_OTHER})

# Collect all .c files from src/ for the simulator, excluding GUI-related files
file(GLOB SIMULATOR_SRC_FILES 
    src/*.c
)

# Exclude GUI files (src/views/*.c) from simulator build
list(FILTER SIMULATOR_SRC_FILES EXCLUDE REGEX "src/views/.*")

# Add the simulator executable (backend only)
add_executable(simulator ${SIMULATOR_SRC_FILES})

# Collect all GUI source files from src/views/
file(GLOB GUI_SRC_FILES src/views/*.c)

# Add the GTK4 test executable (GUI)
add_executable(gtk4-test ${GUI_SRC_FILES})

# Link GTK4 libraries to the GUI executable
target_link_libraries(gtk4-test ${GTK4_LIBRARIES})
