cmake_minimum_required(VERSION 3.10)
project(OS_Simulator C)

# Set C standard
set(CMAKE_C_STANDARD 99)

# Include directories
include_directories(include src)

# Collect all .c files from src/
file(GLOB SRC_FILES src/*.c)

# Exclude "main" binary or other non-C files if necessary
list(REMOVE_ITEM SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/main")

# Add the executable
add_executable(simulator ${SRC_FILES})
