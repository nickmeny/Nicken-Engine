#include "GUI/handlers.h"
#include <stdlib.h>


Node* AddCollision(float screen_width, float screen_height, size_t current_node_count)
{
    //Creat some memory for the node
    Node *node = malloc(sizeof(*node));
    if (!node) return NULL;
    /************ STYLE AND VISUALS FOR THE BUTTON ********/
    
    //start pos
    float start_y = 58.0f;
    float row_height = 25.0f;
    float current_y = start_y + (current_node_count * row_height);
    
    float panel_width = screen_width / 4.0f;
    float margin_left = 10.0f;
    float icon_size = 20.0f;
    float gap = 5.0f;

    //Init the style and the type for the Node
    node->color = RED;
    node->label_text = "Collision Shape";
    node->type = NODE_TYPE_COLLISION;
    node->selected = false;

    //Initialize the collision     
    node->collision.type = COLLISION_NONE;
    node->collision.box_bounds = (Rectangle){ 0 };
    node->collision.circle_center = (Vector2){ 0 };
    node->collision.circle_radius = 0.0f;
    node->collision.has_collision = false;
    //The "red box" besude the label ( only for visual)
    node->rec = (Rectangle){ margin_left, current_y, icon_size, icon_size };
    
    float label_x = margin_left + icon_size + gap;
    float label_width = panel_width - label_x - 10.0f;

    node->label = (Rectangle){ label_x, current_y, label_width, icon_size };

    return node;
}