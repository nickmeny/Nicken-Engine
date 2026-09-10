#pragma once
#include "raylib.h"
extern Vector2 canvas_origin;
extern Rectangle canvas_bounds;

static inline Vector2 ScreenToWorld(Vector2 screen_pos, Vector2 canvas_origin) {
    return (Vector2){
        screen_pos.x - canvas_origin.x,
        screen_pos.y - canvas_origin.y
    };
}

static inline Vector2 WorldToScreen(Vector2 world_pos, Vector2 canvas_origin) {
    return (Vector2){
        world_pos.x + canvas_origin.x,
        world_pos.y + canvas_origin.y
    };
}