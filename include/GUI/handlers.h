#pragma once
#include "raylib.h"
#include <stdlib.h>

typedef enum {
    NODE_TYPE_NONE = 0,
    NODE_TYPE_COLLISION,
    NODE_TYPE_SPRITE,
    NODE_TYPE_MESH
} NodeType;

typedef enum {
    COLLISION_NONE = 0,
    COLLISION_BOX,
    COLLISION_CIRCLE
} CollisionType;

typedef struct 
{
    Rectangle rec;         
    Color color;           
    Rectangle label;       
    char * label_text;  
    NodeType type;     
    bool selected;      


    CollisionType collision_type; 
    Rectangle box_bounds;         
    Vector2 circle_center;        
    float circle_radius;          
    bool has_collision;           
} Node;

Node* AddCollision(float screen_width, float screen_height, size_t current_node_count);