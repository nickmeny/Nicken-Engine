#include <stdio.h>
#include "raylib.h"
#include "mylua.h"
#include "lua_bindings.h"
#include "ECS.h"
#include "CLI.h"
#include "engine_conf.h"
#include "string.h"
#include <stdlib.h>

static int comparer(Pointer a, Pointer b)
{
    const char * str1 = (const char *) a;
    const char* str2 = (const char *)b;
    return strcmp(str1,str2);
}


int main(int argc, char *argv[]) {
    switch(CLI(argc,argv))
    {
        case 0:
            return 0;
        case 1:
            return 1;
        default:
            argv = argv+1;
            break;
    }

    Map map = map_create(comparer,free,free);
    map_set_hash_function(map, hash_string);
    EngineConfig config;
    LoadEngineConfigs(NULL,&config);
    LoadKeyBindings(config.KeyBindingsNameFile,map);

    for(MapNode node = map_first(map);node!=MAP_EOF;node = map_next(map,node))
    {
        printf("Action: %s, Key: %d\n",(char*)map_node_key(node),*((KeyboardKey*)map_node_value(node)));
    }
    
    RegisterMapAction(map);


    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    RegisterEngineFunctions(L);

    if (luaL_dofile(L, argv[1]) != LUA_OK) {
        printf("Error: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }
    SetConfigFlags(FLAG_VSYNC_HINT);
    if (!IsWindowInitialized()) {
        InitWindow(500, 500, "Nicken Default Window");
        SetTargetFPS(config.targetFPS);
    }
    
    Camera2D camera = {0};
    camera.zoom = 1.0f;
    float dt;
    while (!WindowShouldClose()) {
        dt = GetFrameTime();
        
        lua_getglobal(L, "Update");
        lua_pushnumber(L, dt);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            printf("ERROR from Lua: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        ECS_MovementSystem(dt);
        ECS_CollisionSystem(dt);

        BeginDrawing();
        ClearBackground(WHITE);
        ECS_RenderSystem(camera);
            DrawFPS(10,10);
        EndDrawing();
    }

    CloseWindow();
    lua_close(L);
    map_destroy(map);
    return 0;
}
