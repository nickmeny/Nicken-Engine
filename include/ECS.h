/*
API Exposure for the ecs system
*/
#pragma once
#include "ECS_Types.h"

int GetNextFreeID(void);
void ECS_RenderSystem(Camera2D camera);
void ECS_MovementSystem(float dt);
