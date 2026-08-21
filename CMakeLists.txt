cmake_minimum_required(VERSION 3.10)
project(BoubaOS C)

# Specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set build directory
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include/core
    ${PROJECT_SOURCE_DIR}/include/memory
    ${PROJECT_SOURCE_DIR}/include/scheduling
    ${PROJECT_SOURCE_DIR}/include/utils
    ${PROJECT_SOURCE_DIR}/include/commands
)

# Create executable
add_executable(picoshell
    src/commands/echo.c
    src/commands/exec.c
    src/commands/help.c
    src/commands/cd.c
    src/commands/fork.c
    src/commands/ls.c
    src/commands/mkdir.c
    src/commands/touch.c
    src/commands/print.c
    src/commands/quit.c
    src/commands/run.c
    src/commands/set.c
    src/core/shell.c
    src/core/interpreter.c
    src/file_system/file_system.c
    src/memory/memory.c
    src/memory/frame_store.c
    src/memory/variable_store.c
    src/memory/lru.c
    src/scheduling/pcb.c
    src/scheduling/ready_queue.c
    src/scheduling/scheduler.c
    src/utils/badcommand.c
    src/utils/utils.c
)

# Gather user-defined flags
set(FRAME_SIZE "3" CACHE STRING "Size of a single frame in memory")
set(FRAME_STORE_SIZE "300" CACHE STRING "Number of frames in the frame store")
set(VAR_STORE_SIZE "10" CACHE STRING
  "Number of variable entries in the variable memory table")

# Pass values to compilation flags
target_compile_definitions(
  picoshell
    PRIVATE
    FRAME_SIZE=${FRAME_SIZE}
    FRAME_STORE_SIZE=${FRAME_STORE_SIZE}
    VAR_STORE_SIZE=${VAR_STORE_SIZE}
)
