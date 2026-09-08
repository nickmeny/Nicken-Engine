cmake_minimum_required(VERSION 3.14)
project(RaylibLuaJITDeps C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

# ==============================================================================
# 1. LuaJIT
# ==============================================================================
FetchContent_Declare(
    luajit
    GIT_REPOSITORY https://github.com/LuaJIT/LuaJIT.git
    GIT_TAG        v2.1
)
FetchContent_MakeAvailable(luajit)

if(NOT EXISTS "${luajit_SOURCE_DIR}/src/libluajit.a")
    message(STATUS "Compiling LuaJIT library...")
    execute_process(
        COMMAND make -j4
        WORKING_DIRECTORY ${luajit_SOURCE_DIR}/src
        RESULT_VARIABLE LUAJIT_BUILD_RESULT
    )
    if(NOT LUAJIT_BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to build LuaJIT!")
    endif()
endif()

add_library(luajit STATIC IMPORTED GLOBAL)
set_target_properties(luajit PROPERTIES
    IMPORTED_LOCATION "${luajit_SOURCE_DIR}/src/libluajit.a"
    INTERFACE_INCLUDE_DIRECTORIES "${luajit_SOURCE_DIR}/src"
)

# ==============================================================================
# 2. Raylib
# ==============================================================================
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# FetchContent_Declare(
#     clay
#     GIT_REPOSITORY https://github.com/nicbarker/clay.git
#     GIT_TAG        main # Replace with a specific tag or commit hash if preferred
# )
# FetchContent_MakeAvailable(clay)

# # Clay is a header-only library, so create an INTERFACE target to manage headers
# add_library(clay INTERFACE)
# target_include_directories(clay INTERFACE ${clay_SOURCE_DIR})

FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG        5.0
)
FetchContent_MakeAvailable(raygui)

# Δημιουργούμε ένα INTERFACE target για τη Raygui
add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE ${raygui_SOURCE_DIR}/src)