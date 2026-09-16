/*
API Exposure for the ecs system
*/
#pragma once
#include "ECS_Types.h"
#include "Map.h"

void ECS_RenderSystem(Camera2D camera);
void ECS_MovementSystem(float dt);
void ECS_CollisionSystem(float dt);
int CreateEntity(void);
void InitECS(void);
void DestroyEntity(uint32_t entity_id);

