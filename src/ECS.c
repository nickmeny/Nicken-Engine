#include <stdio.h>
#include <inttypes.h>
#include "ECS.h"
#include "rlgl.h"
#include "raymath.h"
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