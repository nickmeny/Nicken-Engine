#pragma once
#include "raylib.h"

typedef enum {
    NODE_TYPE_NONE = 0, //No type
    NODE_TYPE_COLLISION,
    NODE_TYPE_SPRITE,
    NODE_TYPE_MESH
} NodeType;
typedef enum
{
    MESH_TYPE_NONE =0,
    MESH_TYPE_BOX,
    MESH_TYPE_CIRCLE
}MeshType;

//Collision type
typedef enum {
    COLLISION_NONE = 0,
    COLLISION_BOX,
    COLLISION_CIRCLE
} CollisionType;
typedef struct {
    CollisionType type; 
    Rectangle box_bounds;         
    Vector2 circle_center;        
    float circle_radius;          
    bool has_collision;           
} CollisionData;

typedef struct {
    MeshType type;
    Rectangle box_bounds;
    Vector2 circle_center;
    float circle_radius;
    bool has_mesh;
} MeshData;

