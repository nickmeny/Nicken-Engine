/*
API Exposure for the ecs system
*/
#pragma once
#include "ECS_Types.h"
#include "Map.h"

void ECS_RenderSystem(Camera2D camera);
void ECS_MovementSystem(float dt);
void ECS_CollisionSystem(float dt);
void ECS_UpdateAnimationSystem(float dt);
int CreateEntity(void);
void DestroyEntity(uint32_t entity_id);
void ECS_DebugRenderSystem(Camera2D camera);
void InitECS(uint32_t max_entities);
void FreeECS(void);
void PlayAnimationByName(AnimationComponent *anim,const char * name);

