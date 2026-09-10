#include <stdio.h>
#include <inttypes.h>
#include "ECS.h"
#include "rlgl.h"
#include "raymath.h"
#include "math.h"


/*
Here i allocate mamory for the whole struct in data-segment. Because of that i 
escape the problem with stack overflow and also performance issues. If i had allocated memory in Heap 
the memroy will be not continious and the CPU will not get LCache memory. 
Using Tables in data segment it helps CPU to laod a good amount of bytes
in LCache and do the rendering more efficiently
*/
ECS ecs = {0};
static Shader circleShader = { 0 };
static bool shaderLoaded = false;

//Function to get the next free id of the ecs table
int GetNextFreeID(void)
{
    //if the list of free ids is not empty i get form there the id
    if (ecs.free_list_count > 0) {
        ecs.free_list_count--;
        return ecs.free_list_ids[ecs.free_list_count];
    }
    //if the free list is empty i get the next id from the table of entities
    if (ecs.entity_count < MAX_ENTITIES) {
        return ecs.entity_count++;
    }
    //If is not possible to get a ID is mean the engine has reached the MAX_ENTITY number
    return -1; 
}

static void RenderRec(uint32_t mask)
{
    rlBegin(RL_QUADS);
    for (int i = 0; i < ecs.entity_count; i++) 
    {
        if ((ecs.entinty_bitmask[i] & mask) == mask && ecs.mesh[i].type == MESH_RECTANGLE)
        {
            Color c = ecs.mesh[i].color;
            if (c.a == 0) c.a = 255;
            rlColor4ub(c.r, c.g, c.b, c.a);

            float x = ecs.position[i].x;
            float y = ecs.position[i].y;
            float w = ecs.mesh[i].size.x;
            float h = ecs.mesh[i].size.y;

            rlVertex2f(x, y);
            rlVertex2f(x, y + h);
            rlVertex2f(x + w, y + h);
            rlVertex2f(x + w, y);
        }
    }
    rlEnd();
}

static void RenderCircle(uint32_t mask)
{
    BeginShaderMode(circleShader);
    rlBegin(RL_QUADS);
    for (int i = 0; i < ecs.entity_count; i++) 
    {
        if ((ecs.entinty_bitmask[i] & mask) == mask && ecs.mesh[i].type == MESH_CICLE)
        {
            Color c = ecs.mesh[i].color;
            // if (c.a == 0) c.a = 255;
            rlColor4ub(c.r, c.g, c.b, c.a);

            float x = ecs.position[i].x;
            float y = ecs.position[i].y;
            float r = ecs.mesh[i].size.x; // radius
            //The frist is the UV for the shader and the second is to create a rec, aeound the center of the circle with width 2r
            rlTexCoord2f(0.0f, 0.0f); rlVertex2f(x - r, y - r);
            rlTexCoord2f(0.0f, 1.0f); rlVertex2f(x - r, y + r);
            rlTexCoord2f(1.0f, 1.0f); rlVertex2f(x + r, y + r);
            rlTexCoord2f(1.0f, 0.0f); rlVertex2f(x + r, y - r);
        }
    }
    rlEnd();
    EndShaderMode();
}

void ECS_RenderSystem(Camera2D camera)
{
    //load the shader only one time
    if (!shaderLoaded) {
        circleShader = LoadShader(0, "circle.fs");
        shaderLoaded = true;
    }

    uint32_t mask = COMPOMENT_POSITION | COMPOMENT_MESH;

    BeginMode2D(camera);
        RenderCircle(mask);
        RenderRec(mask);
    EndMode2D();
}

void ECS_MovementSystem(float dt)
{
    uint32_t mask = COMPOMENT_VELOCITY | COMPOMENT_POSITION;
    for(int i=0;i<ecs.entity_count;i++)
    {
        if((ecs.entinty_bitmask[i] & mask) != mask) continue;
        ecs.position[i].x = ecs.position[i].x+ecs.velocity[i].vx * dt;
        ecs.position[i].y = ecs.position[i].y + ecs.velocity[i].vy* dt;
    }
}


// ================================
//      COLLISION SYSTEM
//=================================
static SpatialGrid grid;


static inline int Hash2D(int cellX,int cellY)
{
    unsigned int h1 = (unsigned int)cellX * 73856093;
    unsigned int h2 = (unsigned int)cellY * 19349663;
    unsigned int hash = h1 ^ h2;
    return (int)(hash & (HASH_TABLE_SIZE-1));
}
static void SpacilaGridClear(void)
{
    for(int i=0;i<HASH_TABLE_SIZE;i++)
    {
        grid.buckets[i] = -1;
    }
}

static void SpatialGridInsert(int entity_id,Vector2 pos)
{
    int cellX = (int)floorf(pos.x/GRID_CELL_SIZE);
    int cellY = (int)floorf(pos.y/GRID_CELL_SIZE);
    int hash = Hash2D(cellX,cellY);
    grid.spatial_next[entity_id] = grid.buckets[hash];
    grid.buckets[hash] = entity_id;
}

void ECS_CollisionSystem(float dt)
{
    (void)dt;
    SpacilaGridClear();
    ecs.collision_event_count = 0;
    uint32_t mask = COMPONENT_COLLISION | COMPOMENT_POSITION;
    for (int i = 0; i < ecs.entity_count; i++)
    {
        if ((ecs.entinty_bitmask[i] & mask) == mask)
        {
            Vector2 pos = {
                .x = ecs.position[i].x + ecs.collision[i].offsets.x,
                .y = ecs.position[i].y + ecs.collision[i].offsets.y
            };
            SpatialGridInsert(i, pos);
        }
    }
    for (int a = 0; a < ecs.entity_count; a++)
    {
        if ((ecs.entinty_bitmask[a] & mask) != mask) continue;
        if (ecs.collision[a].is_static) continue; 

        Vector2 pos_a = (Vector2){
            .x = ecs.position[a].x + ecs.collision[a].offsets.x,
            .y = ecs.position[a].y + ecs.collision[a].offsets.y
        };

        int centerCellX = (int)floorf(pos_a.x / GRID_CELL_SIZE);
        int centerCellY = (int)floorf(pos_a.y / GRID_CELL_SIZE);

        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                int hash = Hash2D(centerCellX + dx, centerCellY + dy);
                int b = grid.buckets[hash];

                while (b != -1)
                {
                    if (a != b)
                    {
                        if ((ecs.collision[a].collision_mask & ecs.collision[b].collision_layer) != 0)
                        {
                            Rectangle recA = {
                                .x = ecs.position[a].x + ecs.collision[a].offsets.x,
                                .y = ecs.position[a].y + ecs.collision[a].offsets.y,
                                .width = ecs.collision[a].size.x,
                                .height = ecs.collision[a].size.y
                            };

                            Rectangle recB = {
                                .x = ecs.position[b].x + ecs.collision[b].offsets.x,
                                .y = ecs.position[b].y + ecs.collision[b].offsets.y,
                                .width = ecs.collision[b].size.x,
                                .height = ecs.collision[b].size.y
                            };

                            if (CheckCollisionRecs(recA, recB))
                            {
                                // Αποθήκευση Event για τη Lua
                                if (ecs.collision_event_count < 256) {
                                    ecs.frame_collisions[ecs.collision_event_count++] = (CollisionEvent){ a, b };
                                }
                                float overlapX1 = (recA.x + recA.width) - recB.x;
                                float overlapX2 = (recB.x + recB.width) - recA.x;
                                float overlapY1 = (recA.y + recA.height) - recB.y;
                                float overlapY2 = (recB.y + recB.height) - recA.y;

                                float overlapX = (overlapX1 < overlapX2) ? overlapX1 : overlapX2;
                                float overlapY = (overlapY1 < overlapY2) ? overlapY1 : overlapY2;

                                if (overlapX < overlapY)
                                {
                                    if (overlapX1 < overlapX2) {
                                        ecs.position[a].x -= overlapX; // Hit right wall -> Push Left
                                    } else {
                                        ecs.position[a].x += overlapX; // Hit left wall -> Push Right
                                    }
                                    if ((ecs.entinty_bitmask[a] & COMPOMENT_VELOCITY) == COMPOMENT_VELOCITY) {
                                        ecs.velocity[a].vx = 0;
                                    }
                                }
                                else
                                {
                                    if (overlapY1 < overlapY2) {
                                        ecs.position[a].y -= overlapY; // Hit bottom wall -> Push Up
                                    } else {
                                        ecs.position[a].y += overlapY; // Hit top wall -> Push Down
                                    }
                                    if ((ecs.entinty_bitmask[a] & COMPOMENT_VELOCITY) == COMPOMENT_VELOCITY) {
                                        ecs.velocity[a].vy = 0;
                                    }
                                }
                            }
                        }
                    }
                    b = grid.spatial_next[b];
                }
            }
        }
    }
}