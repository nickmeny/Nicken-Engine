/*
Clean ECS header file for types and thinks that use in ECS.
*/
#pragma once

#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MATCH_COLOR(str, name, raylib_color) \
    if (strcmp(str, name) == 0) return raylib_color;

/*
Here is some defines for compoment bitmask.
The system is working using ECS method.
So each entity will have some compoment ( for example position).
To know which compoment has we using bitmask
*/

#define MAX_ENTITIES 100000
#define MAX_COLLISION_EVENTS 2000
#define INVALID_INDEX 0xFFFFFFFF

typedef struct 
{
    float x;
    float y;
}PositionComponent;

typedef struct 
{
    float vx;
    float vy;
}VelocityComponent;

typedef struct
{
    uint32_t texture_id;
    int width;
    int height;
    int render_layer;
}SpriteComponent;

typedef enum
{
    MESH_NONE=0,
    MESH_RECTANGLE,
    MESH_CIRCLE
}MeshType;

typedef enum
{
    COLLISION_NONE=0,
    COLLISION_REC,
    COLLISION_CIRCLE
}CollisionType;

typedef struct 
{
    MeshType type;
    Vector2 size;
    Color color;
    int render_layer;
}MeshComponent;

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

#define DEFINE_SPARSE_POOL(Type, Name) \
typedef struct { \
    Type data[MAX_ENTITIES]; \
    uint32_t packed_to_entity[MAX_ENTITIES]; \
    uint32_t sparse[MAX_ENTITIES]; \
    uint32_t count; \
} Name##Pool; \
\
static inline void Init##Name##Pool(Name##Pool* pool) { \
    pool->count = 0; \
    memset(pool->sparse, 0xFF, sizeof(pool->sparse)); \
} \
\
static inline void Add##Name(Name##Pool* pool, uint32_t entity_id, Type comp) { \
    if (entity_id >= MAX_ENTITIES) return;\
    uint32_t existing_index = pool->sparse[entity_id]; \
    if (existing_index != INVALID_INDEX) { \
        pool->data[existing_index] = comp; \
        return; \
    } \
    uint32_t new_index = pool->count; \
    pool->data[new_index] = comp; \
    pool->packed_to_entity[new_index] = entity_id; \
    pool->sparse[entity_id] = new_index; \
    pool->count++; \
} \
\
static inline Type* Get##Name(Name##Pool* pool, uint32_t entity_id) { \
    if (entity_id >= MAX_ENTITIES) return NULL; \
    uint32_t index = pool->sparse[entity_id]; \
    if (index >= pool->count) return NULL; /* Ασφαλής έλεγχος αντί για == INVALID_INDEX */ \
    return &pool->data[index]; \
} \
\
static inline void Remove##Name(Name##Pool* pool, uint32_t entity_id) { \
    if (entity_id >= MAX_ENTITIES) return; \
    uint32_t index_to_remove = pool->sparse[entity_id]; \
    if (index_to_remove == INVALID_INDEX) return; \
    uint32_t last_index = pool->count - 1; \
    uint32_t last_entity = pool->packed_to_entity[last_index]; \
    pool->data[index_to_remove] = pool->data[last_index]; \
    pool->packed_to_entity[index_to_remove] = last_entity; \
    pool->sparse[last_entity] = index_to_remove; \
    pool->sparse[entity_id] = INVALID_INDEX; \
    pool->count--; \
}

DEFINE_SPARSE_POOL(PositionComponent, Position)
DEFINE_SPARSE_POOL(VelocityComponent, Velocity)
DEFINE_SPARSE_POOL(MeshComponent, Mesh)
DEFINE_SPARSE_POOL(CollisionComponent, Collision)
DEFINE_SPARSE_POOL(SpriteComponent, Sprite)

typedef struct 
{
    int free_list_ids[MAX_ENTITIES];
    int free_list_count;
    PositionPool position;
    VelocityPool velocity;
    SpritePool sprite;
    MeshPool mesh;
    CollisionPool collision;
    CollisionEvent frame_collisions[MAX_COLLISION_EVENTS];
    int collision_event_count;
} ECS;

extern ECS ecs;