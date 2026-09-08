#include "GUI/gui_toolbar.h"
#include "raygui.h"

//collision toolbar buttons 
static ToolButton collision_tools[] = {
    { 1, "#119#Box", false },
    { 2, "#120#Circle", false },
    { 3, "#28#Delete", false }
};

//draw the toolbar and return if a button inside it was clicked
int DrawToolbar(Toolbar *bar) {
    //check if the bar must be shown
    if (!bar || !bar->active || bar->buttonCount == 0) return -1;

    //the clicked id set to -1 ( no buttons have bean clickd yet)
    int clicked_id = -1;
    float current_x = bar->bounds.x;
    float btn_h = 25.0f, spacing = 5.0f;
    //Draw the buttons 
    for (int i = 0; i < bar->buttonCount; i++) {
        float text_width = GuiGetTextWidth(bar->buttons[i].text); 
        float btn_w = text_width + 30.0f;
        Rectangle btn_rec = { current_x, bar->bounds.y, btn_w, btn_h };
        //if a button has bean pressed
        if (GuiButton(btn_rec, bar->buttons[i].text)) {
            clicked_id = bar->buttons[i].id; //get the id
            bar->buttons[i].is_active = true; //set the "active" attribute to true
        }
        current_x += btn_w + spacing;
    }
    return clicked_id;
}
/// @brief it enable or disable the tool bar and init it with the currect values for each node
/// @param bar the toolbar
/// @param selected_node  the current node
/// @param screen_width  the screen width
void UpdateToolbarForNode(Toolbar *bar, Node *selected_node, float screen_width) {
    //if the node is null ( no node is selected) make the toolbar disapear
    if (!selected_node) {
        bar->active = false;
        bar->buttonCount = 0;
        bar->buttons = NULL;
        return;
    }
    //else

    bar->active = true;
    bar->bounds = (Rectangle){ 70, 0, screen_width - 70, 25 };
    //See what node is selceted
    switch (selected_node->type) {
        case NODE_TYPE_COLLISION:
            bar->buttons = collision_tools; //the collision toolbar set up
            bar->buttonCount = sizeof(collision_tools) / sizeof(collision_tools[0]);
            break;
        default:
            bar->active = false;
            break;
    }
}