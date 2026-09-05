BUILD_DIR      = build
OBJ_DIR        = obj
INCLUDE_FOLDER = include
TARGET         = engine
SRC_FOLDER     = src

RAYLIB_LIB     = $(BUILD_DIR)/_deps/raylib-build/raylib/libraylib.a
LUAJIT_LIB     = $(BUILD_DIR)/_deps/luajit-src/src/libluajit.a

SRCS           = $(shell find $(SRC_FOLDER) -name '*.c')
OBJS           = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

INCLUDE_DIRS   = -I$(BUILD_DIR)/_deps/luajit-src/src \
                 -I$(BUILD_DIR)/_deps/raylib-src/src \
				 -I$(BUILD_DIR)/_deps/clay-src \
                 -I$(INCLUDE_FOLDER)

LIB_DIRS       = -L$(BUILD_DIR)/_deps/raylib-build/raylib

# Link directly against static libraries (no shared library lookup needed)
LIBS           = -lraylib $(LUAJIT_LIB) -lm -lpthread -ldl

CC             = gcc
CFLAGS         = -Wall -Wextra -O3 -g

.PHONY: all deps engine run clean clean-all

all: engine

deps: $(RAYLIB_LIB) $(LUAJIT_LIB)

$(RAYLIB_LIB) $(LUAJIT_LIB):
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

engine: deps $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIB_DIRS) $(LIBS) -o $(TARGET)

run: engine
	./$(TARGET) run main.lua
init: engine
	./$(TARGET) init

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

clean-all:
	rm -rf $(BUILD_DIR) $(OBJ_DIR) $(TARGET)