#include <stdlib.h>
#include "raylib.h"
#include "Vector.h"
#include "GUI/handlers.h"
#include "GUI/gui_toolbar.h"
#include "GUI/gui_components.h"
#include "GUI/collision_tool.h"
#include "GUI/utils.h"
//GCC flags to skip the warnings beacause the ryagui has a lot of warnigns
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#pragma GCC diagnostic pop

Rectangle canvas_bounds = { 0 };
Vector2 canvas_origin = { 0 };

void InitCanvas()
{
        float panel_width = GetScreenWidth() / 4.0f;
        float top_bar_height = 25.0f;

        canvas_bounds = (Rectangle){
            .x = panel_width,
            .y = top_bar_height,
            .width = GetScreenWidth() - panel_width,
            .height = (float)GetScreenHeight() - top_bar_height
        };

        canvas_origin =(Vector2){.x=canvas_bounds.x +canvas_bounds.width/2,.y=canvas_bounds.y+canvas_bounds.height/2};
}

//The destroy func for the vector
static void DestroyNode(Pointer value)
{
    free((Node *)value);
}

void init_editor(void)
{
    //Create the vector for the pop up elements
    Vector entities = vector_create(0, DestroyNode);

    //Initialize the window
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Modular Engine Editor");
    SetTargetFPS(60);

    GuiLoadStyle("styles/cyber/style_cyber.rgs");
    Font customFont = LoadFontEx("styles/cyber/Inter-Medium.ttf", 18, NULL, 0);
    SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);
    GuiSetFont(customFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
    //The pop up window is not shown in the start
    bool pop_up = false;
    //Tool bar hide
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
            int action_tool_id = DrawToolbar(&active_toolbar); //get the toolbar action
            if (action_tool_id != -1 && selected_node) //if a button of the toolbar is pressed
            {
                vert_count = 0;
                switch (selected_node->type)
                {
                    case NODE_TYPE_COLLISION:
                        HandleCollisionToolAction(selected_node, action_tool_id, &current_active_tool, &active_toolbar, screen_width);
                        break;

                    case NODE_TYPE_SPRITE:
                        break;

                    case NODE_TYPE_MESH:
                        break;

                    default:
                        break;
                }       
            }

            // Input only when we dont have pop up
            if (selected_node && current_active_tool != 0)
            {
                switch (selected_node->type)
                {
                case NODE_TYPE_COLLISION:
                    UpdateCollisionInput(selected_node, current_active_tool, mouse_pos, &vert_count, pts);
                    break;
                default:
                    break;
                }
            }
        }
        

        // RENDERING LOGIC
        BeginDrawing();
        ClearBackground(GRAY);
        InitCanvas();
        // Draw the nodes in canvas
        for (int i = 0; i < (int)vector_size(entities); i++)
        {
            Node *node = (Node *)vector_get_at(entities, i);
            if (node->type == NODE_TYPE_COLLISION)
            {
                DrawCollisionCanvas(node);
            }
        }

        // Preview for collision ( only if the pop up is not active)
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
        DrawCircle(canvas_origin.x,canvas_origin.y,1.0f,RED);
        EndDrawing();
    }

    vector_destroy(entities);
    UnloadFont(customFont);
    CloseWindow();
}