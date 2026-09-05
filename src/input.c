#include "raylib.h"
#include "Map.h"

bool is_action_down(Map map,const char* action)
{
    int key = *(int*)map_find(map,(Pointer)action);
    return IsKeyPressed(key);
}