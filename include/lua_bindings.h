#pragma once
#include "mylua.h"
#include <stdbool.h>
#include "input.h"

bool IsWindowInitialized(void);
void RegisterEngineFunctions(lua_State *L);
int C_CreateEntity(lua_State *L);
int C_Window_Initialize(lua_State * L);
void RegisterMapAction(Map map);
