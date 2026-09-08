#pragma once
#include "raylib.h"
#include "handlers.h"

typedef struct ToolButton {
    int id;                 
    const char *text;       
    bool is_active;
} ToolButton;

typedef struct Toolbar {
    ToolButton *buttons;    
    int buttonCount;        
    bool active;            
    Rectangle bounds;       
} Toolbar;

int DrawToolbar(Toolbar *bar);
void UpdateToolbarForNode(Toolbar *bar, Node *selected_node, float screen_width);