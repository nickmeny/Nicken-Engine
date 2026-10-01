BUILD_DIR      = build
BUILD_DIR_WIN  = build_win
OBJ_DIR        = obj
OBJ_DIR_WIN    = obj_win
INCLUDE_FOLDER = include
TARGET         = engine
TARGET_WIN     = engine.exe
SRC_FOLDER     = src

RAYLIB_LIB     = $(BUILD_DIR)/_deps/raylib-build/raylib/libraylib.a
LUAJIT_LIB     = $(BUILD_DIR)/_deps/luajit-src/src/libluajit.a

RAYLIB_LIB_WIN = $(BUILD_DIR_WIN)/_deps/raylib-build/raylib/libraylib.a
# Διορθώθηκε: libluajit.a αντί για lua51.lib
LUAJIT_LIB_WIN = $(BUILD_DIR_WIN)/_deps/luajit-src/src/libluajit.a

SRCS           = $(shell find $(SRC_FOLDER) -name '*.c')
OBJS           = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))
OBJS_WIN       = $(patsubst %.c, $(OBJ_DIR_WIN)/%.o, $(SRCS))

INCLUDE_DIRS   = -I$(BUILD_DIR)/_deps/luajit-src/src \
                 -I$(BUILD_DIR)/_deps/raylib-src/src \
                 -I$(BUILD_DIR)/_deps/clay-src \
                 -I$(BUILD_DIR)/_deps/raygui-src/src \
                 -I$(INCLUDE_FOLDER)

INCLUDE_DIRS_WIN = -I$(BUILD_DIR_WIN)/_deps/luajit-src/src \
                     -I$(BUILD_DIR_WIN)/_deps/raylib-src/src \
                     -I$(BUILD_DIR_WIN)/_deps/clay-src \
                     -I$(BUILD_DIR_WIN)/_deps/raygui-src/src \
                     -I$(INCLUDE_FOLDER)

LIB_DIRS       = -L$(BUILD_DIR)/_deps/raylib-build/raylib
LIB_DIRS_WIN   = -L$(BUILD_DIR_WIN)/_deps/raylib-build/raylib

# Link flags
LIBS           = -lraylib $(LUAJIT_LIB) -lX11 -lm -lpthread -ldl
LIBS_WIN       = -static -lraylib $(LUAJIT_LIB_WIN) -lopengl32 -lgdi32 -lwinmm -lshell32 -luser32

CC             = gcc
CC_WIN         = x86_64-w64-mingw32-gcc
CFLAGS         = -Wall -Wextra -O3 -g

.PHONY: all deps engine windows deps-win run clean clean-all editor

all: engine

deps: $(RAYLIB_LIB) $(LUAJIT_LIB)

$(RAYLIB_LIB) $(LUAJIT_LIB):
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR)

deps-win: $(RAYLIB_LIB_WIN) $(LUAJIT_LIB_WIN)

$(RAYLIB_LIB_WIN) $(LUAJIT_LIB_WIN):
	cmake -B $(BUILD_DIR_WIN) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_SYSTEM_NAME=Windows \
		-DCMAKE_C_COMPILER=$(CC_WIN) \
		-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
	cmake --build $(BUILD_DIR_WIN)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(OBJ_DIR_WIN)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC_WIN) $(CFLAGS) $(INCLUDE_DIRS_WIN) -c $< -o $@

engine: deps $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIB_DIRS) $(LIBS) -o $(TARGET)

windows: deps-win $(OBJS_WIN)
	$(CC_WIN) $(CFLAGS) $(OBJS_WIN) $(LIB_DIRS_WIN) $(LIBS_WIN) -o $(TARGET_WIN)

run: engine
	./$(TARGET) run main.lua

init: engine
	./$(TARGET) init

editor: engine
	./$(TARGET) editor

clean:
	rm -rf $(OBJ_DIR) $(OBJ_DIR_WIN) $(TARGET) $(TARGET_WIN)

clean-all:
	rm -rf $(BUILD_DIR) $(BUILD_DIR_WIN) $(OBJ_DIR) $(OBJ_DIR_WIN) $(TARGET) $(TARGET_WIN)