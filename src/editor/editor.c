#include <stdlib.h>
#include "raylib.h"
#include "Vector.h"
#include "GUI/handlers.h"
#include "GUI/gui_toolbar.h"
#include "GUI/gui_components.h"
#include "GUI/collision_tool.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#pragma GCC diagnostic pop

static void DestroyNode(Pointer value)
{
    free((Node *)value);
}

void init_editor(void)
{
    Vector entities = vector_create(0, DestroyNode);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Modular Engine Editor");
    SetTargetFPS(60);

    GuiLoadStyle("styles/cherry/style_cherry.rgs");
    GuiSetFont(GetFontDefault());

    bool pop_up = false;
    Toolbar active_toolbar = {0};
    Node *selected_node = NULL;

    int current_active_tool = 0;
    Vector2 pts[2];
    int vert_count = 0;

    while (!WindowShouldClose())
    {
        float screen_width = (float)GetScreenWidth();
        Vector2 mouse_pos = GetMousePosition();

        //UPDATE LOGIC
        if (!pop_up)
        {
            int action_tool_id = DrawToolbar(&active_toolbar);
            if (action_tool_id != -1)
            {
                if (action_tool_id == 3 && selected_node)
                {
                    selected_node->has_collision = false;
                    selected_node->collision_type = COLLISION_NONE;
                    current_active_tool = 0;
                    
                    // Reset toolbar
                    UpdateToolbarForNode(&active_toolbar, selected_node, screen_width);
                }
                else
                {
                    current_active_tool = action_tool_id;
                }
                vert_count = 0;
            }

            // Input handling μόνο όταν ΔΕΝ έχουμε ανοιχτό pop-up
            if (selected_node && current_active_tool != 0)
            {
                switch (selected_node->type)
                {
                case NODE_TYPE_COLLISION:
                    UpdateCollisionInput(selected_node, current_active_tool, mouse_pos, screen_width, &vert_count, pts);
                    break;
                default:
                    break;
                }
            }
        }

        // RENDERING LOGIC
        BeginDrawing();
        ClearBackground(GRAY);

        // Draw the nodes in canvas
        for (int i = 0; i < (int)vector_size(entities); i++)
        {
            Node *node = (Node *)vector_get_at(entities, i);
            if (node->type == NODE_TYPE_COLLISION)
            {
                DrawCollisionCanvas(node);
            }
        }

        // Preview ( only if the pop up is not active)
        if (!pop_up && selected_node && selected_node->type == NODE_TYPE_COLLISION)
        {
            DrawCollisionPreview(current_active_tool, vert_count, pts, mouse_pos);
        }

        // Top bar with export
        GuiPanel((Rectangle){ 0, 0, screen_width, 25 }, NULL);
        if (GuiButton((Rectangle){ 0, 0, 60, 25 }, "Export")) {
            TraceLog(LOG_INFO, "Export clicked");
        }

        // Toolbar
        DrawToolbar(&active_toolbar);

        //Left panel with compoments
        DrawEntityComponentsPanel(screen_width, &pop_up, entities, &selected_node, &active_toolbar);
        DrawNodeListUI(entities, &selected_node, &active_toolbar, screen_width);

        EndDrawing();
    }

    vector_destroy(entities);
    CloseWindow();
}