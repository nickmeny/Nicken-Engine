#pragma once
#include "raylib.h"
#include "handlers.h"

void UpdateCollisionInput(Node* node, int tool_id, Vector2 mouse_pos, float screen_width, int *vert_count, Vector2 pts[]);
void DrawCollisionPreview(int tool_mode, int vert_count, Vector2 pts[], Vector2 mouse_pos);
void DrawCollisionCanvas(const Node* node);