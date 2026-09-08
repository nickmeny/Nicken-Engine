#include "GUI/collision_tool.h"
#include "raymath.h"
#include <math.h>

void UpdateCollisionInput(Node* node, int tool_id, Vector2 mouse_pos, float screen_width, int *vert_count, Vector2 pts[]) {
    if (!node) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        *vert_count = 0;
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse_pos.x > (screen_width / 4.0f) && mouse_pos.y > 25) {
        pts[*vert_count] = mouse_pos;
        (*vert_count)++;

        if (tool_id == 1 && *vert_count == 2) { // BOX
            float x = (pts[0].x < pts[1].x) ? pts[0].x : pts[1].x;
            float y = (pts[0].y < pts[1].y) ? pts[0].y : pts[1].y;
            float w = fabsf(pts[1].x - pts[0].x);
            float h = fabsf(pts[1].y - pts[0].y);

            node->collision_type = COLLISION_BOX;
            node->box_bounds = (Rectangle){ x, y, w, h };
            node->has_collision = true;
            *vert_count = 0;
        }
        else if (tool_id == 2 && *vert_count == 2) { // CIRCLE
            float radius = Vector2Distance(pts[0], pts[1]);

            node->collision_type = COLLISION_CIRCLE;
            node->circle_center = pts[0];
            node->circle_radius = radius;
            node->has_collision = true;
            *vert_count = 0;
        }
    }
}

void DrawCollisionCanvas(const Node* node) {
    if (!node || !node->has_collision) return;

    Color border_color = node->selected ? YELLOW : BLUE;

    if (node->collision_type == COLLISION_BOX) {
        DrawRectangleRec(node->box_bounds, Fade(BLUE, 0.4f));
        DrawRectangleLinesEx(node->box_bounds, 2.0f, border_color);
    } 
    else if (node->collision_type == COLLISION_CIRCLE) {
        DrawCircleV(node->circle_center, node->circle_radius, Fade(BLUE, 0.4f));
        DrawCircleLines((int)node->circle_center.x, (int)node->circle_center.y, node->circle_radius, border_color);
    }
}

void DrawCollisionPreview(int tool_mode, int vert_count, Vector2 pts[], Vector2 mouse_pos) {
    if (vert_count != 1) return;

    if (tool_mode == 1) { // Box Preview
        float px = (pts[0].x < mouse_pos.x) ? pts[0].x : mouse_pos.x;
        float py = (pts[0].y < mouse_pos.y) ? pts[0].y : mouse_pos.y;
        float pw = fabsf(mouse_pos.x - pts[0].x);
        float ph = fabsf(mouse_pos.y - pts[0].y);

        Rectangle preview_rec = { px, py, pw, ph };
        DrawRectangleRec(preview_rec, Fade(BLUE, 0.3f));
        DrawRectangleLinesEx(preview_rec, 1.5f, RED);
        DrawCircleV(pts[0], 4, RED);
    } 
    else if (tool_mode == 2) { // Circle Preview
        float r = Vector2Distance(pts[0], mouse_pos);
        DrawCircleV(pts[0], r, Fade(BLUE, 0.3f));
        DrawCircleLines((int)pts[0].x, (int)pts[0].y, r, RED);
        DrawLineV(pts[0], mouse_pos, RED);
        DrawCircleV(pts[0], 4, RED);
    }
}