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

    //For key binds
    Map map_keybinds = map_create(comparer,free,free);
    map_set_hash_function(map_keybinds, hash_string);
    
    EngineConfig config;
    
    LoadEngineConfigs(NULL,&config);
    LoadKeyBindings(config.KeyBindingsNameFile,map_keybinds);
    
    RegisterMapAction(map_keybinds);
    InitWindow(500, 500, "Nicken Default Window");

    InitECS(MAX_ENTITIES);


    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    RegisterEngineFunctions(L);

    if (luaL_dofile(L, argv[1]) != LUA_OK) {
        printf("Error: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    SetTargetFPS(config.targetFPS);
    SetConfigFlags(FLAG_VSYNC_HINT);
    lua_getglobal(L, "Init");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            printf("ERROR in Lua Init: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }
    
    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0.0f, 0.0f };
    camera.offset = (Vector2){ 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 2.0f;
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
        ECS_UpdateAnimationSystem(dt);
        BeginDrawing();
        ClearBackground(DARKGRAY);
        ECS_DebugRenderSystem(camera);
        ECS_RenderSystem(camera);
        DrawFPS(10,10);
        EndDrawing();
    }

    FreeECS();
    CloseWindow();
    lua_close(L);
    map_destroy(map_keybinds);
    UnloadTextureCache();
    return 0;
}
