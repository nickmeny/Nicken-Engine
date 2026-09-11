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
#define COMPONENT_COLLISION (1<<4)
#define COMPOMENT_MESH (1<<5)
#define MAX_ENTITIES 10000
#define MAX_COLLISION_EVENTS ((MAX_ENTITIES)/2)

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

typedef enum
{
    COLLISION_NONE=0,
    COLLISION_REC,
    COLLISION_CICLE
}CollisionType;

typedef struct 
{
    MeshType type;
    Vector2 size;
    Color color;
}MeshCompoment;

typedef struct
{
    CollisionType type;
    Vector2 size;
    Vector2 offsets;
    uint8_t collision_layer;
    uint8_t collision_mask;
    bool is_trigger;
    bool is_static;
}CollisionComponent;

typedef struct {
    int entity_a;
    int entity_b;
} CollisionEvent;



typedef struct 
{
    int free_list_ids[MAX_ENTITIES];
    int free_list_count;
    uint32_t entinty_bitmask[MAX_ENTITIES];
    PositionCompoment position[MAX_ENTITIES];
    VelocityCompoment velocity[MAX_ENTITIES];
    SpriteCompoment sprite[MAX_ENTITIES];
    MeshCompoment mesh[MAX_ENTITIES];
    CollisionComponent collision[MAX_ENTITIES];
    CollisionEvent frame_collisions[MAX_COLLISION_EVENTS];
    int entity_count;
    int collision_event_count;
} ECS;

extern ECS ecs;