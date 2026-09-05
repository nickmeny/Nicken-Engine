/*
Clean ECS header file for types and thinks that use in ECS.
*/
#pragma once

#include "raylib.h"
#include "inttypes.h"

#define MATCH_COLOR(str, name, raylib_color) \
    if (strcmp(str, name) == 0) return raylib_color;

/*
Here is some defines for compoment bitmask.
The system is working using ECS method.
So each entity will have some compoment ( for example position).
To know which compoment has we using bitmask
*/
#define COMPOMENT_NONE 0
#define COMPOMENT_POSITION (1<<0)
#define COMPOMENT_VELOCITY (1<<1)
#define COMPOMENT_SPRITE ( 1<<2)
#define COMPOMET_PHYSICS (1<<3)
#define COMPOMENT_MESH (1<<4)

#define MAX_ENTITIES 10000

typedef struct 
{
    float x;
    float y;
}PositionCompoment;

typedef struct 
{
    float vx;
    float vy;
}VelocityCompoment;

typedef struct
{
    Texture2D texture;
    int width;
    int height;
}SpriteCompoment;

typedef enum
{
    MESH_NONE=0,
    MESH_RECTANGLE,
    MESH_CICLE
}MeshType;

typedef struct 
{
    MeshType type;
    Vector2 size;
    Color color;
}MeshCompoment;


typedef struct 
{
    int free_list_ids[MAX_ENTITIES];
    int free_list_count;
    uint32_t entinty_bitmask[MAX_ENTITIES];
    PositionCompoment position[MAX_ENTITIES];
    VelocityCompoment velocity[MAX_ENTITIES];
    SpriteCompoment sprite[MAX_ENTITIES];
    MeshCompoment mesh[MAX_ENTITIES];
    int entity_count;
} ECS;

extern ECS ecs;