#include "GUI/gui_toolbar.h"
#include "raygui.h"

static ToolButton collision_tools[] = {
    { 1, "#119#Box", false },
    { 2, "#120#Circle", false },
    { 3, "#28#Delete", false }
};

int DrawToolbar(Toolbar *bar) {
    if (!bar || !bar->active || bar->buttonCount == 0) return -1;

    int clicked_id = -1;
    float btn_w = 60.0f, btn_h = 25.0f, spacing = 5.0f;

    for (int i = 0; i < bar->buttonCount; i++) {
        Rectangle btn_rec = { bar->bounds.x + (i * (btn_w + spacing)), bar->bounds.y, btn_w, btn_h };
        if (GuiButton(btn_rec, bar->buttons[i].text)) {
            clicked_id = bar->buttons[i].id;
            bar->buttons[i].is_active = true;
        }
    }
    return clicked_id;
}

void UpdateToolbarForNode(Toolbar *bar, Node *selected_node, float screen_width) {
    if (!selected_node) {
        bar->active = false;
        bar->buttonCount = 0;
        bar->buttons = NULL;
        return;
    }

    bar->active = true;
    bar->bounds = (Rectangle){ 70, 0, screen_width - 70, 25 };

    switch (selected_node->type) {
        case NODE_TYPE_COLLISION:
            bar->buttons = collision_tools;
            bar->buttonCount = sizeof(collision_tools) / sizeof(collision_tools[0]);
            break;
        default:
            bar->active = false;
            break;
    }
}