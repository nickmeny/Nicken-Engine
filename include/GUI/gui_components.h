#pragma once
#include "raylib.h"
#include "Vector.h"
#include "handlers.h"
#include "GUI/gui_toolbar.h"

void DrawEntityComponentsPanel(float screen_width, bool *pop_up, Vector vec, Node **selected_node, Toolbar *active_toolbar);
void DrawNodeListUI(Vector entities, Node **selected_node, Toolbar *active_toolbar, float screen_width);