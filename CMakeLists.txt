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

if(WIN32 AND CMAKE_CROSSCOMPILING)
    # Arguments ως λίστα για να μην προστεθούν διπλά quotes από το CMake
    set(LUAJIT_BUILD_ARGS -j4 HOST_CC=gcc CROSS=x86_64-w64-mingw32- TARGET_SYS=Windows BUILDMODE=static)
    set(LUAJIT_LIB_NAME "libluajit.a")
else()
    set(LUAJIT_BUILD_ARGS -j4 BUILDMODE=static)
    set(LUAJIT_LIB_NAME "libluajit.a")
endif()

if(NOT EXISTS "${luajit_SOURCE_DIR}/src/${LUAJIT_LIB_NAME}")
    message(STATUS "Compiling LuaJIT library for target: ${CMAKE_SYSTEM_NAME}...")
    execute_process(
        COMMAND make ${LUAJIT_BUILD_ARGS}
        WORKING_DIRECTORY ${luajit_SOURCE_DIR}/src
        RESULT_VARIABLE LUAJIT_BUILD_RESULT
    )
    if(NOT LUAJIT_BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to build LuaJIT!")
    endif()
endif()

add_library(luajit STATIC IMPORTED GLOBAL)
set_target_properties(luajit PROPERTIES
    IMPORTED_LOCATION "${luajit_SOURCE_DIR}/src/${LUAJIT_LIB_NAME}"
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

# ==============================================================================
# 3. Raygui
# ==============================================================================
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG        5.0
)
FetchContent_MakeAvailable(raygui)

add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE ${raygui_SOURCE_DIR}/src)