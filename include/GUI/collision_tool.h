#pragma once
#include "raylib.h"
#include "GUI/handlers.h"
#include "GUI/gui_toolbar.h"

void UpdateCollisionInput(Node* node, int tool_id, Vector2 mouse_pos, float screen_width, int *vert_count, Vector2 pts[]);
void DrawCollisionPreview(int tool_mode, int vert_count, Vector2 pts[], Vector2 mouse_pos);
void DrawCollisionCanvas(const Node* node);
void HandleCollisionToolAction(Node *node, int tool_id, int *current_active_tool, Toolbar *toolbar, float screen_width);