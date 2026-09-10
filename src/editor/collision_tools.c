#include "GUI/collision_tool.h"
#include "raymath.h"
#include <math.h>
#include "GUI/utils.h"

/// @brief  This function is to initialize the Schema of the collision 
/// @param node  thge collision node
/// @param tool_id  the button id
/// @param mouse_pos the mous position
/// @param screen_width the screen width
/// @param vert_count  //the counter for the table of points
/// @param pts //table of points
void UpdateCollisionInput(Node* node, int tool_id, Vector2 mouse_pos, int *vert_count, Vector2 pts[]) {
    if (!node) return;
    //If the rught click is pressed it resets the draw func
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        *vert_count = 0;
        return;
    }
    //id the left mouse clicked and the mouse pos is outside of the UI box
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse_pos,canvas_bounds)) {
        Vector2 world_click = ScreenToWorld(mouse_pos,canvas_origin);
        TraceLog(LOG_INFO, "[CLICK %d] Screen: (%.1f, %.1f) | World (Local 0,0): (%.1f, %.1f)", 
                 *vert_count, mouse_pos.x, mouse_pos.y, world_click.x, world_click.y);
        pts[*vert_count] = world_click;//Put the coords in the points table
        (*vert_count)++;//add one to the vertex counter
        
        //if the tool is for box and the vertex counter is 2 ( 2 points) 
        if (tool_id == 1 && *vert_count == 2) {
            //the x for the top left corenr is the smallest x of the 2 points 
            float x = (pts[0].x < pts[1].x) ? pts[0].x : pts[1].x;
            float y = (pts[0].y < pts[1].y) ? pts[0].y : pts[1].y; //the y for the top left corener is the smallest y of the 2 points
            float w = fabsf(pts[1].x - pts[0].x); //the absulute value of the distance between the 2 points in the x axis
            float h = fabsf(pts[1].y - pts[0].y); //the absulute value of the distance between the 2 points in the y axis
            
            //the type is BOX (also here i use the collision attribute of the union) 
            node->collision.type = COLLISION_BOX;
            node->collision.box_bounds = (Rectangle){ x, y, w, h }; //the bounds
            node->collision.has_collision = true; //it has now collision
            *vert_count = 0; //reset the vert count
            TraceLog(LOG_INFO, "===> BOX CREATED <===");
            TraceLog(LOG_INFO, "     Local Offset X: %.2f | Y: %.2f", x, y);
            TraceLog(LOG_INFO, "     Size Width    : %.2f | Height: %.2f", w, h);
        }//esle if the tool is for circle
        else if (tool_id == 2 && *vert_count == 2) { // CIRCLE
            float radius = Vector2Distance(pts[0], pts[1]); //calculate the radius 

            node->collision.type = COLLISION_CIRCLE;
            node->collision.circle_center = pts[0]; //the center is the first "click"
            node->collision.circle_radius = radius;
            node->collision.has_collision = true;
            *vert_count = 0;
            TraceLog(LOG_INFO, "===> CIRCLE CREATED <===");
            TraceLog(LOG_INFO, "     Local Center X: %.2f | Y: %.2f", pts[0].x, pts[0].y);
            TraceLog(LOG_INFO, "     Radius        : %.2f", radius);
        }
    }
}

/// @brief Draw the collision for the node
/// @param node 
void DrawCollisionCanvas(const Node* node) {
    //if the node is not collision or the collision must not be seen, it returns
    if (!node || node->type !=NODE_TYPE_COLLISION || !node->collision.has_collision) return; 
    //Make the collision outline to be yellow if the node is selected
    Color border_color = node->selected ? YELLOW : BLUE;

    //if the collision is box
    if (node->collision.type == COLLISION_BOX) {
        Vector2 world_pos = { node->collision.box_bounds.x, node->collision.box_bounds.y };
        Vector2 screen_pos = WorldToScreen(world_pos, canvas_origin);
        Rectangle screen_rect = {
            screen_pos.x,
            screen_pos.y,
            node->collision.box_bounds.width,
            node->collision.box_bounds.height
        };
        //Draw the rec
        DrawRectangleRec(screen_rect, Fade(BLUE, 0.4f));
        DrawRectangleLinesEx(screen_rect, 2.0f, border_color);
    } 
    //else draw the cyrcle
    else if (node->collision.type == COLLISION_CIRCLE) {
        Vector2 screen_center = WorldToScreen(node->collision.circle_center, canvas_origin);
        DrawCircleV(screen_center, node->collision.circle_radius, Fade(BLUE, 0.4f));
        DrawCircleLines((int)screen_center.x, (int)screen_center.y, node->collision.circle_radius, border_color);
    }
}

/// @brief This function is to draw the collision when the user draw it
/// @param tool_mode the tool id
/// @param vert_count the counter for the points table
/// @param pts the points
/// @param mouse_pos  the mouse positioin
void DrawCollisionPreview(int tool_mode, int vert_count, Vector2 pts[], Vector2 mouse_pos) {
    if (vert_count != 1) return;
    Vector2 start_screen = WorldToScreen(pts[0], canvas_origin);
    if (tool_mode == 1) { // Box Preview
        //here if the point is more close to the (0,0) i take it as the starting point, else the px is the mouse pos
        float px = (start_screen.x < mouse_pos.x) ? start_screen.x : mouse_pos.x;
        float py = (start_screen.y < mouse_pos.y) ? start_screen.y : mouse_pos.y;
        float pw = fabsf(mouse_pos.x - start_screen.x);
        float ph = fabsf(mouse_pos.y - start_screen.y);

        //The previw rectangle
        Rectangle preview_rec = { px, py, pw, ph };
        DrawRectangleRec(preview_rec, Fade(BLUE, 0.3f));
        DrawRectangleLinesEx(preview_rec, 1.5f, RED);
        DrawCircleV(pts[0], 4, RED); //here i draw a circle for the starting point ( it is the first click pos)
    } 
    else if (tool_mode == 2) { // Circle Preview
        float r = Vector2Distance(start_screen, mouse_pos); //the radius is the distance from the initial point to the cyrremnt mous pos
        DrawCircleV(start_screen, r, Fade(BLUE, 0.3f)); //fill the circle with the blue color
        DrawCircleLines((int)start_screen.x, (int)start_screen.y, r, RED); //the circle outline
        DrawLineV(start_screen, mouse_pos, RED); //The radius line form the center to the mouse pos
        DrawCircleV(start_screen, 4, RED); //draw the center
    }
}

/// @brief Function that handle the toolbar actions
/// @param node the collision node
/// @param tool_id the button id
/// @param current_active_tool the tool that is active
/// @param toolbar the tool bar
/// @param screen_width screen width
void HandleCollisionToolAction(Node *node, int tool_id, int *current_active_tool, Toolbar *toolbar, float screen_width) {
    if (!node || node->type != NODE_TYPE_COLLISION) return; //if the node is not a collision type
    //switch the buttons
    switch (tool_id) {
        case 1: // Box Tool, i dont put extra logix here because by default the tool is the box
        case 2: // Circle Tool
            *current_active_tool = tool_id; //put the id
            break;

        case 3: // Delete Action
            node->collision.has_collision = false;
            node->collision.type = COLLISION_NONE;
            *current_active_tool = 0; // disable the current tool
            
            // Updtaet the ui
            UpdateToolbarForNode(toolbar, node, screen_width);
            break;

        default:
            break;
    }
}