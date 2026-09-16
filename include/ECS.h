/*
API Exposure for the ecs system
*/
#pragma once
#include "ECS_Types.h"
#include "Map.h"

int GetNextFreeID(void);
void ECS_RenderSystem(Camera2D camera);
void ECS_MovementSystem(float dt);
void ECS_CollisionSystem(float dt);
Map ECSTextureMap(void);
void ECS_SpriteRenderSystem(Map texture_map);
