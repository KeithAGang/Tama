cmake_minimum_required(VERSION 3.20)

project(Tama LANGUAGES CXX)

# --- Default Build Type ---
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
endif()

# --- C++23 Standard Setup ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Tooling ---
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Cross-Platform Debugging Options ---
option(ENABLE_DEBUG_TOOLS "Enable address sanitizer and debug info" OFF)

if(ENABLE_DEBUG_TOOLS)
    if(MSVC)
        add_compile_options(/fsanitize=address /Zi)
    else()
        add_compile_options(-fsanitize=address -g -O0)
        add_link_options(-fsanitize=address)
    endif()
endif()

# --- Sub-projects ---
add_subdirectory(src/internals/Migrator)
add_subdirectory(src/internals/Config)
add_subdirectory(src/internals/Commands)
add_subdirectory(src/internals/Db)
add_subdirectory(src/internals/Parser)

# --- Main Executable ---
add_subdirectory(src)
