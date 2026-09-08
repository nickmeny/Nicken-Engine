#include "GUI/collision_tool.h"
#include "raymath.h"
#include <math.h>

/// @brief  This function is to initialize the Schema of the collision 
/// @param node  thge collision node
/// @param tool_id  the button id
/// @param mouse_pos the mous position
/// @param screen_width the screen width
/// @param vert_count  //the counter for the table of points
/// @param pts //table of points
void UpdateCollisionInput(Node* node, int tool_id, Vector2 mouse_pos, float screen_width, int *vert_count, Vector2 pts[]) {
    if (!node) return;
    //If the rught click is pressed it resets the draw func
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        *vert_count = 0;
        return;
    }
    //id the left mouse clicked and the mouse pos is outside of the UI box
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse_pos.x > (screen_width / 4.0f) && mouse_pos.y > 25) {
        pts[*vert_count] = mouse_pos; //Put the coords in the points table
        (*vert_count)++; //add one to the vertex counter
        
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
        }//esle if the tool is for circle
        else if (tool_id == 2 && *vert_count == 2) { // CIRCLE
            float radius = Vector2Distance(pts[0], pts[1]); //calculate the radius 

            node->collision.type = COLLISION_CIRCLE;
            node->collision.circle_center = pts[0]; //the center is the first "click"
            node->collision.circle_radius = radius;
            node->collision.has_collision = true;
            *vert_count = 0;
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
        //Draw the rec
        DrawRectangleRec(node->collision.box_bounds, Fade(BLUE, 0.4f));
        DrawRectangleLinesEx(node->collision.box_bounds, 2.0f, border_color);
    } 
    //else draw the cyrcle
    else if (node->collision.type == COLLISION_CIRCLE) {
        DrawCircleV(node->collision.circle_center, node->collision.circle_radius, Fade(BLUE, 0.4f));
        DrawCircleLines((int)node->collision.circle_center.x, (int)node->collision.circle_center.y, node->collision.circle_radius, border_color);
    }
}

/// @brief This function is to draw the collision when the user draw it
/// @param tool_mode the tool id
/// @param vert_count the counter for the points table
/// @param pts the points
/// @param mouse_pos  the mouse positioin
void DrawCollisionPreview(int tool_mode, int vert_count, Vector2 pts[], Vector2 mouse_pos) {
    if (vert_count != 1) return;

    if (tool_mode == 1) { // Box Preview
        //here if the point is more close to the (0,0) i take it as the starting point, else the px is the mouse pos
        float px = (pts[0].x < mouse_pos.x) ? pts[0].x : mouse_pos.x;
        float py = (pts[0].y < mouse_pos.y) ? pts[0].y : mouse_pos.y;
        //the distance 
        float pw = fabsf(mouse_pos.x - pts[0].x);
        float ph = fabsf(mouse_pos.y - pts[0].y);
        //The previw rectangle
        Rectangle preview_rec = { px, py, pw, ph };
        DrawRectangleRec(preview_rec, Fade(BLUE, 0.3f));
        DrawRectangleLinesEx(preview_rec, 1.5f, RED);
        DrawCircleV(pts[0], 4, RED); //here i draw a circle for the starting point ( it is the first click pos)
    } 
    else if (tool_mode == 2) { // Circle Preview
        float r = Vector2Distance(pts[0], mouse_pos); //the radius is the distance from the initial point to the cyrremnt mous pos
        DrawCircleV(pts[0], r, Fade(BLUE, 0.3f)); //fill the circle with the blue color
        DrawCircleLines((int)pts[0].x, (int)pts[0].y, r, RED); //the circle outline
        DrawLineV(pts[0], mouse_pos, RED); //The radius line form the center to the mouse pos
        DrawCircleV(pts[0], 4, RED); //draw the center
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