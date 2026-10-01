/*
Clean ECS header file for types and thinks that use in ECS.
*/
#pragma once

#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

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
    int texture_h;
    int texture_w;
    int width;
    int height;
    int render_layer;
    float x;
    float y;
    bool flip_x;
    bool flip_y;
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

typedef struct 
{
    float frame_time;
    float frame_duration;
    int frame_number;
    int current_frame;
    int frame_width;
    int frame_height;
}AnimationComponent;


//Here is a VRY VERY BIG MACRO. Its job is to Auto create the repeated functions of the Pools( Add,Remove,Init,Get) And the structs.
#define DEFINE_SPARSE_POOL(Type, Name) \
typedef struct { \
    Type* data; /*This is the data of the actual compomnets but it is dense*/ \ 
    uint32_t *packed_to_entity; /*This table saves the entities id in the indexes of the data. For example if the data[0] has the data of the entity with id 500: packed_to_entity[0]=500*/ \
    uint32_t *sparse;/*This table is the reverse of the packed_to_entity. In the example above this table in index 500 has value of 0*/ \
    uint32_t count; \
} Name##Pool; \
\
static inline void Init##Name##Pool(Name##Pool* pool,uint32_t max_entities)/*init the pool*/ { \
    pool->count = 0; \
    pool->data = (Type*)malloc(sizeof(Type)*max_entities);\
    pool->packed_to_entity = (uint32_t*)malloc(sizeof(uint32_t)*max_entities);\
    pool->sparse= (uint32_t*)malloc(sizeof(uint32_t)*max_entities);\
    memset(pool->sparse, 0xFF, sizeof(uint32_t)*max_entities); \
} \
\
static inline void Add##Name(Name##Pool* pool, uint32_t entity_id, Type comp) { \
    if (entity_id >= MAX_ENTITIES) return;\
    uint32_t existing_index = pool->sparse[entity_id];/*get the datat table index*/ \
    if (existing_index != INVALID_INDEX) { \
        pool->data[existing_index] = comp; \
        return; \
    } \
    uint32_t new_index = pool->count; \
    pool->data[new_index] = comp; \
    pool->packed_to_entity[new_index] = entity_id; /*put in the index of the packed the actual id*/ \
    pool->sparse[entity_id] = new_index;/*put in index of the actual id the data id*/ \
    pool->count++; \
} \
\
static inline Type* Get##Name(Name##Pool* pool, uint32_t entity_id) { \
    if (entity_id >= MAX_ENTITIES) return NULL; \
    uint32_t index = pool->sparse[entity_id]; \
    if (index >= pool->count) return NULL; \
    return &pool->data[index]; \
} \
\
static inline void Remove##Name(Name##Pool* pool, uint32_t entity_id) { \
    if (entity_id >= MAX_ENTITIES) return; \
    uint32_t index_to_remove = pool->sparse[entity_id];/*get the data id*/ \
    if (index_to_remove == INVALID_INDEX) return; \
    uint32_t last_index = pool->count - 1; \
    uint32_t last_entity = pool->packed_to_entity[last_index];/*Get the enity id*/ \
    pool->data[index_to_remove] = pool->data[last_index];/*swap the remove index with the last index*/ \
    pool->packed_to_entity[index_to_remove] = last_entity; /*put in the packed array in index of the old the new aka previews last entity*/ \
    pool->sparse[last_entity] = index_to_remove;/*update the spare table*/ \
    pool->sparse[entity_id] = INVALID_INDEX; \
    pool->count--; \
}\
\
static inline void Free##Name##Pool(Name##Pool* pool){\
    free(pool->data);\
    free(pool->packed_to_entity);\
    free(pool->sparse);\
}

//Create the POOLS
DEFINE_SPARSE_POOL(PositionComponent, Position)
DEFINE_SPARSE_POOL(VelocityComponent, Velocity)
DEFINE_SPARSE_POOL(MeshComponent, Mesh)
DEFINE_SPARSE_POOL(CollisionComponent, Collision)
DEFINE_SPARSE_POOL(SpriteComponent, Sprite)
DEFINE_SPARSE_POOL(AnimationComponent,Animation)
typedef struct 
{
    int* free_list_ids;
    int free_list_count;
    uint32_t max_entities;
    PositionPool position;
    VelocityPool velocity;
    SpritePool sprite;
    AnimationPool animation;
    MeshPool mesh;
    CollisionPool collision;
    CollisionEvent frame_collisions[MAX_COLLISION_EVENTS];
    int collision_event_count;
} ECS;

extern ECS ecs;