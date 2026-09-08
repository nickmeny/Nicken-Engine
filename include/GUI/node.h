#pragma once
#include "raylib.h"
#include <stdbool.h>
#include "GUI/node_types.h"


typedef struct Node {
    Rectangle rec;         
    Color color;           
    Rectangle label;       
    char *label_text;  
    NodeType type;     
    bool selected;      
    union {
        CollisionData collision;
        MeshData mesh;
    };
} Node;