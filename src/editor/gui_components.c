// GUI/gui_components.c
#include "GUI/gui_components.h"
#include "raygui.h"
//A macro to auto make the buttons aligne
#define GUI_POPUP_BUTTON(rec, index, text) \
    GuiButton((Rectangle){ (rec).x + 30, (rec).y + 35 + ((index) * 40), (rec).width - 60, 30 }, (text))

typedef struct {
    char * name;
    Node* (*Handler)(float screen_width, float screen_height, size_t current_node_count);
} Compomnets;

//This func is to draw the panle where the Components are in the popup
void DrawEntityComponentsPanel(float screen_width, bool *pop_up, Vector vec, Node **selected_node, Toolbar *active_toolbar) {
    GuiPanel((Rectangle){0, 25, screen_width / 4, GetScreenHeight() - 25}, NULL);
    if (GuiButton((Rectangle){screen_width / 4 - 60, 27, 60, 30}, "Add")) {
        *pop_up = true;
    }

    if (*pop_up) {
        const Compomnets components[] = {
            {"Collision Shape", AddCollision},
            {"Sprite", NULL},
            {"Mesh", NULL}
        };

        int button_count = sizeof(components) / sizeof(components[0]);
        float total_popup_height = 35.0f + (button_count * 40.0f) + 10.0f;
        //The rec for the add button
        Rectangle popup_rec = { GetScreenWidth() / 2.0f - 150.0f, GetScreenHeight() / 2.0f - (total_popup_height / 2.0f), 300.0f, total_popup_height };

        if (GuiWindowBox(popup_rec, "Add component")) {
            *pop_up = false;
        }
        //Make the pop up to automatically get the size
        for (int i = 0; i < button_count; i++) {
            if (GUI_POPUP_BUTTON(popup_rec, i, components[i].name)) {
                if (components[i].Handler != NULL) {
                    //call the handle function
                    Node* new_node = components[i].Handler(screen_width, GetScreenHeight(), vector_size(vec));
                    *pop_up = false;
                    //insert the node in the vector
                    if (new_node) {
                        vector_insert_last(vec, new_node);

                        // --- DESELECT LOGIC ---
                        //Because when the user add a enity, we dont want to draw tbehind it the current node
                        //if it has a node selected, make the selected node false and then null
                        if (*selected_node) {
                            (*selected_node)->selected = false;
                        }
                        *selected_node = NULL; 
                        //update the ui toolbar
                        UpdateToolbarForNode(active_toolbar, NULL, screen_width);
                    }
                }
            }
        }
    }
}

/// @brief 
/// @param entities the verctor with the components
/// @param selected_node  the current selected node
/// @param active_toolbar the tool bar
/// @param screen_width  the width o f the screen
void DrawNodeListUI(Vector entities, Node **selected_node, Toolbar *active_toolbar, float screen_width) {
    for (int i = 0; i < (int)vector_size(entities); i++) {
        Node* node = (Node*)vector_get_at(entities, i);
        if (node) {
            Color border_color = node->selected ? YELLOW : BLACK; //id the user is editing the node make the borkeder color yellow
            DrawRectangleRec(node->rec, node->color);
            DrawRectangleLinesEx(node->rec, 2, border_color);
            if (GuiLabelButton(node->label, node->label_text)) {
                if (*selected_node) (*selected_node)->selected = false; //if the user click the entity, it diselect the current entity 
                //and select the other entity
                node->selected = true;
                *selected_node = node;
                //updatet the tool bar 
                UpdateToolbarForNode(active_toolbar, *selected_node, screen_width);
            }
        }
    }
}