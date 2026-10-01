#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "ECS.h"
#include "rlgl.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>


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

//this struct is for the rendering. Is a way to get rid of the classic bitmask and use a better way with pools
typedef struct
{
    uint64_t key; //32 bits layer
    uint32_t entity_id;
    float w;
    float h;
    float x, y;
    float anim_x,anim_y;
    float anim_w,anim_h;
    float tex_w, tex_h;
    bool flip_x; 
    bool flip_y;
}RenderCommand;

typedef struct 
{
    RenderCommand *commands;
    RenderCommand *tmp_commands;
    uint32_t count;
    uint32_t capacity;
}RenderQueue;

static void InitRenderQueue(RenderQueue *queue,uint32_t capacity)
{
    queue->capacity=capacity;
    queue->commands = (RenderCommand*)malloc(sizeof(RenderCommand)*capacity);
    queue->tmp_commands = (RenderCommand*)malloc(sizeof(RenderCommand)*capacity);
    queue->count=0;
}

static void FreeRenderQueue(RenderQueue *queue)
{
    if(queue->commands) free(queue->commands);
    if(queue->tmp_commands) free(queue->tmp_commands);
    queue->commands = NULL;
    queue->tmp_commands = NULL;
    queue->count = 0;
    queue->capacity=0;
}

static inline uint64_t MakeRenderKey(uint8_t layer, float y_pos, uint32_t resource_id, uint8_t is_mesh) {
    float clamped_y = y_pos + 10000.0f;
    if (clamped_y < 0.0f) clamped_y = 0.0f;
    if (clamped_y > 16777215.0f) clamped_y = 16777215.0f;
    uint32_t depth = (uint32_t)clamped_y;
    uint64_t layer_part = ((uint64_t)(layer & 0xFF)) << 56;
    uint64_t depth_part = ((uint64_t)(depth & 0xFFFFFF)) << 32;
    uint64_t res_part   = ((uint64_t)(resource_id & 0x7FFFFFFF)) << 1;
    uint64_t type_part  = (uint64_t)is_mesh & 0x01;

    return layer_part | depth_part | res_part | type_part;
}

static void RadixSort(RenderQueue* queue, uint32_t max_bits)
{
    uint32_t count = queue->count;
    if (count < 2) return;

    RenderCommand *src = queue->commands;
    RenderCommand *dst = queue->tmp_commands;

    // Only iterate up to max_bits instead of 64
    for (int shift = 0; shift < max_bits; shift += 8) {
        uint32_t histogram[256] = {0};

        for (uint32_t i = 0; i < count; i++) {
            uint8_t byte = (src[i].key >> shift) & 0xFF;
            histogram[byte]++;
        }

        uint32_t offset[256] = {0};
        for (int i = 1; i < 256; i++) {
            offset[i] = offset[i - 1] + histogram[i - 1];
        }

        for (uint32_t i = 0; i < count; i++) {
            uint8_t byte = (src[i].key >> shift) & 0xFF;
            dst[offset[byte]++] = src[i];
        }

        RenderCommand *temp = src;
        src = dst;
        dst = temp;
    }

    if (src != queue->commands) {
        RenderCommand *temp = queue->commands;
        queue->commands = queue->tmp_commands;
        queue->tmp_commands = temp;
    }
}



static RenderQueue render_queue = {0};
//Function fot qsort
int CompareRenderCommands(const void *a, const void *b) {
    uint64_t keyA = ((RenderCommand*)a)->key;
    uint64_t keyB = ((RenderCommand*)b)->key;
    if (keyA < keyB) return -1;
    if (keyA > keyB) return 1;
    return 0;
}

void InitECS(uint32_t max_entities)
{
    memset(&ecs, 0, sizeof(ECS));
    // Cleare all the ecs;
    ecs.max_entities = max_entities;
    ecs.free_list_count = max_entities;
    ecs.free_list_ids = (int*)malloc(sizeof(int)*max_entities);
    if(ecs.free_list_ids==NULL){printf("ERROR: MALLOC FAIL");exit(1);}
    for (int i = 0; i < max_entities; i++) {
        ecs.free_list_ids[i] = i;
    }
    // Here Initilize all the spare pools
    InitPositionPool(&ecs.position,max_entities);
    InitCollisionPool(&ecs.collision,max_entities);
    InitVelocityPool(&ecs.velocity,max_entities);
    InitSpritePool(&ecs.sprite,max_entities);
    InitMeshPool(&ecs.mesh,max_entities);
    InitAnimationPool(&ecs.animation,max_entities);
    InitRenderQueue(&render_queue,max_entities);
}

void FreeECS(void)
{
    free(ecs.free_list_ids);
    FreePositionPool(&ecs.position);
    FreeVelocityPool(&ecs.velocity);
    FreeSpritePool(&ecs.sprite);
    FreeMeshPool(&ecs.mesh);
    FreeCollisionPool(&ecs.collision);
    FreeAnimationPool(&ecs.animation);
    FreeRenderQueue(&render_queue);
}

//THis method allows the create entity to be in O(1). Becasue the system add the free id in the end and it does zero swifts
int CreateEntity(void) {
    if (ecs.free_list_count <= 0) { //if the list is full
        TraceLog(LOG_ERROR, "ECS: Out of entity IDs!");
        return -1;
    }
    //Remove from the end of the free list
    ecs.free_list_count--;
    int entity_id = ecs.free_list_ids[ecs.free_list_count]; //get the last id;
    return entity_id;
}

void DestroyEntity(uint32_t entity_id) {
    if (entity_id >= MAX_ENTITIES) return; //if the id is out of bounds return

    // Delete the entity from all the SparePools
    RemovePosition(&ecs.position, entity_id);
    RemoveVelocity(&ecs.velocity, entity_id);
    RemoveSprite(&ecs.sprite, entity_id);
    RemoveMesh(&ecs.mesh, entity_id);
    RemoveCollision(&ecs.collision, entity_id);
    RemoveAnimation(&ecs.animation,entity_id);
    // Add the id in the free list
    if (ecs.free_list_count < MAX_ENTITIES) {
        ecs.free_list_ids[ecs.free_list_count] = entity_id;
        ecs.free_list_count++;
    }
}

//This function is only for the Animations
//TODO: Update this function so it can support the Multy-rows sprite sheets and no loop animations
void ECS_UpdateAnimationSystem(float dt)
{
    for(uint32_t i = 0; i < ecs.animation.count; i++)
    {
        //Get the animation component
        AnimationComponent *anim = &ecs.animation.data[i];
        anim->frame_time += dt; //increase the frame time by the delta time

        if(anim->frame_time >= anim->frame_duration) //if the frame time is bigger that the duration
        {
            anim->frame_time -= anim->frame_duration; // It resets the frame time. I sub the duration and not just put it in 0f beacuse like that i fix the lag problems
            anim->current_frame = (anim->current_frame + 1) % anim->frame_number; //Here i calculate the new frame. The % is for wehen the anim is going toi the end, return to the start
            
            uint32_t entity_id = ecs.animation.packed_to_entity[i]; //get the id
            SpriteComponent *sprite = GetSprite(&ecs.sprite, entity_id); //get the sprite
            //Get the new crop box. It updates the the texture offsets.
            if(sprite) {
                sprite->x = (float)(anim->current_frame * anim->frame_width);
                sprite->y = 0.0f;
            }
        }
    }
}

//Here is the new Render System. Here i have combane the the sprite and the mesh rendering all in once. I have use the Pools instead the bitmask. so i have almost zero 
//cache misses and also better batch rendering
void ECS_RenderSystem(Camera2D camera)
{

    // load the shader one time so the gpu can do the batch rendering
    if (!shaderLoaded)
    {
        circleShader = LoadShader(0, "shapes.fs");
        shaderLoaded = true;
    }
    render_queue.count = 0;
    //Here i "fetch" all the sprites components. Because i have the Pools i can iritate only the enetitys the have the sprite and not just "continue" in 
    //loop, sth that creates a lot of cache misses
    for (uint32_t i = 0; i < ecs.sprite.count; i++)
    {
        uint32_t entity_id = ecs.sprite.packed_to_entity[i];
        SpriteComponent *sprite = &ecs.sprite.data[i];
        if (sprite->texture_id == 0) continue;

        PositionComponent *pos = GetPosition(&ecs.position, entity_id);
        if (!pos) continue;

        if (render_queue.count >= render_queue.capacity) break;

        uint64_t key = MakeRenderKey(sprite->render_layer, pos->y, sprite->texture_id, 0);
        uint32_t idx = render_queue.count;

        AnimationComponent *anim = GetAnimation(&ecs.animation, entity_id);

        //Frame size from the texture
        float crop_w = (anim && anim->frame_width > 0) ? (float)anim->frame_width : sprite->width;
        float crop_h = (anim && anim->frame_height > 0) ? (float)anim->frame_height : sprite->height;

        //The size in the monitor
        // If the user has provide a size, we using this.
        // If not or if the  sprite->width isι 0/same with the sheet,using the  crop_w!
        float render_w = (sprite->width > 0.0f && sprite->width != sprite->texture_w) ? sprite->width : crop_w;
        float render_h = (sprite->height > 0.0f && sprite->height != sprite->texture_h) ? sprite->height : crop_h;
        float sheet_w = (sprite->texture_w > 0.0f) ? sprite->texture_w : crop_w;
        float sheet_h = (sprite->texture_h > 0.0f) ? sprite->texture_h : crop_h;

        render_queue.commands[idx] = (RenderCommand){
            .key = key,
            .entity_id = entity_id,
            .w = render_w,       // On screen width
            .h = render_h,       // On screen height
            .x = pos->x,
            .y = pos->y,
            .anim_x = sprite->x, 
            .anim_y = sprite->y, 
            .anim_w = crop_w,   
            .anim_h = crop_h,   
            .tex_w = sheet_w,   
            .tex_h = sheet_h,
            .flip_x = sprite->flip_x,
            .flip_y = sprite->flip_y
            };
            render_queue.count++;
    }
    //Here i do the same as the sprites but for the Meshes
    for (uint32_t i = 0; i < ecs.mesh.count; i++)
    {
        uint32_t entity_id = ecs.mesh.packed_to_entity[i];
        MeshComponent *mesh = &ecs.mesh.data[i];

        PositionComponent *pos = GetPosition(&ecs.position, entity_id);
        if(!pos) continue;
        if (render_queue.count >= render_queue.capacity) break;
        float y_pos = pos ? pos->y : 0.0f;

        // Resource ID = 0 για τα meshes
        uint64_t key = MakeRenderKey(mesh->render_layer, y_pos, 0, 1);

        uint32_t idx = render_queue.count;
        render_queue.commands[idx] = (RenderCommand){
            .key = key,
            .entity_id = entity_id,
            .w = mesh->size.x,
            .h = mesh->size.y,
            .x = pos->x,
            .y = pos->y
        };
        render_queue.count++;
    }
    //If no render commands return from the func
    if (render_queue.count == 0)
        return;
    //quick sort the commands with the keys
    RadixSort(&render_queue,64);
    //Here is starting the actuall rendering
    BeginMode2D(camera);
    BeginShaderMode(circleShader);
    uint32_t current_tex = (uint32_t)-1; // The current texture is -1
    bool in_batch = false; // is a switch to know when the gpu has to flash the shader 
    //for all the render commands
    for (uint32_t i = 0; i < render_queue.count; i++)
    {
        RenderCommand *cmd = &render_queue.commands[i];
        uint32_t entity_id = cmd->entity_id; //get the id
        uint64_t key = cmd->key; //get the key
        bool is_mesh = (key & 1); //If is a mesh means the last beat is 1. e.x. 1011(mesh) & 0001 = 0001(true) 1010(sprite)&0001 =0000(false)

        uint32_t tex_id = is_mesh ? 0 : (uint32_t)((key >> 1) & 0x7FFFFFFF); //Here is one more trick i do. The number 0x7FFFFFFF it has 31 ones. The first
        //think i do is to swift the number 1 bit right to throw out the mesh/sprite bit. after that i have a number like
        // 0[32bits layer][31 bits id]. I apply the mask of 0x7FFFFFFF so i "cancel" the bits after the 31st bit.

        float x = cmd->x;
        float y = cmd->y;
        // if (!pos) //if there aren't any just keep
        //     continue;
        if (tex_id != current_tex || !in_batch) //if the current texture is not the text id and is not in batch
        {
            if (in_batch) //if it has texture in , flush it to create a new batch
                rlEnd();

            //Set the texture and start the batch
            current_tex = tex_id; 
            rlSetTexture(current_tex);
            rlBegin(RL_QUADS);
            in_batch = true;
        }

        //For Spirtes 
        if (!is_mesh)
        {
            // SPRITE: Standard UVs 
            float tw = (cmd->tex_w > 0.0f) ? cmd->tex_w : cmd->anim_w;
            float th = (cmd->tex_h > 0.0f) ? cmd->tex_h : cmd->anim_h;

            float u0 = cmd->anim_x / tw;
            float v0 = cmd->anim_y / th;
            float u1 = (cmd->anim_x + cmd->anim_w) / tw;
            float v1 = (cmd->anim_y + cmd->anim_h) / th;
            if (cmd->flip_x) {
                float tmp = u0;
                u0 = u1;
                u1 = tmp;
            }

    //VERTICAL FLIP (SWAP V0 and V1) ---
    if (cmd->flip_y) {
        float tmp = v0;
        v0 = v1;
        v1 = tmp;
    }
            rlColor4ub(255, 255, 255, 255);
            
            rlTexCoord2f(u0, v0); rlVertex2f(x, y);
            rlTexCoord2f(u0, v1); rlVertex2f(x, y + cmd->h);
            rlTexCoord2f(u1, v1); rlVertex2f(x + cmd->w, y + cmd->h);
            rlTexCoord2f(u1, v0); rlVertex2f(x + cmd->w, y);
        }
        else //for meshes
        {
            MeshComponent *mesh = GetMesh(&ecs.mesh, entity_id);
            Color c = mesh->color;
            if (c.a == 0)
                c.a = 255;
            rlColor4ub(c.r, c.g, c.b, c.a);

            if (mesh->type == MESH_RECTANGLE)
            {
                // RECTANGLE: paramenters y + 10.0f to let now the shader is a rec
                float w = mesh->size.x;
                float h = mesh->size.y;

                rlTexCoord2f(0.0f, 10.0f);
                rlVertex2f(x, y);
                rlTexCoord2f(0.0f, 11.0f);
                rlVertex2f(x, y + h);
                rlTexCoord2f(1.0f, 11.0f);
                rlVertex2f(x + w, y + h);
                rlTexCoord2f(1.0f, 10.0f);
                rlVertex2f(x + w, y);
            }
            else if (mesh->type == MESH_CIRCLE)
            {
                // CIRCLE: UVs  in [-1.0, 1.0] but with y-offset 100.0f
                float r = mesh->size.x;

                rlTexCoord2f(-1.0f, -1.0f + 100.0f);
                rlVertex2f(x - r, y - r);
                rlTexCoord2f(-1.0f, 1.0f + 100.0f);
                rlVertex2f(x - r, y + r);
                rlTexCoord2f(1.0f, 1.0f + 100.0f);
                rlVertex2f(x + r, y + r);
                rlTexCoord2f(1.0f, -1.0f + 100.0f);
                rlVertex2f(x + r, y - r);
            }
        }
    }
    if (in_batch) //stop all the prev batches
        rlEnd();
    EndShaderMode();
    EndMode2D();
    rlSetTexture(0);
}

void FreeRenderSystem(void) {
    if (shaderLoaded) {
        UnloadShader(circleShader);
        shaderLoaded = false;
    }
}

void ECS_MovementSystem(float dt)
{
    for(int i=0;i<ecs.velocity.count;i++)
    {
        uint32_t entity_id = ecs.velocity.packed_to_entity[i];
        PositionComponent *pos = GetPosition(&ecs.position,entity_id);
        if(!pos) continue;
        pos->x += ecs.velocity.data[i].vx * dt;
        pos->y += ecs.velocity.data[i].vy * dt;
    }
}


// ================================
//      COLLISION SYSTEM
//=================================
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"
#pragma GCC diagnostic pop
//Here i loop the entities and i use the cute_c2 lib to detect collision. Maybe it can be otpimised but in the futer
//TODO: OPTIMIZE THE CODE
void ECS_CollisionSystem(float dt) {
    (void)dt;
    
    //Cleare the collision of the prev frame
    ecs.collision_event_count = 0;

    uint32_t total_colliders = ecs.collision.count;

    //Loop only the entities with collision
    for (uint32_t i = 0; i < total_colliders; i++) 
    {
        uint32_t entityA = ecs.collision.packed_to_entity[i];
        CollisionComponent *colA = &ecs.collision.data[i];

        //Look up in the osition O(1)
        PositionComponent *posA = GetPosition(&ecs.position, entityA);
        if (!posA) continue; // if the entity has no pos just continiue to the next entity

        //loop fpr the others colliders ( i+1 to not iritate the same pairs)
        for (uint32_t j = i + 1; j < total_colliders; j++) 
        {
            uint32_t entityB = ecs.collision.packed_to_entity[j];
            CollisionComponent *colB = &ecs.collision.data[j];

            PositionComponent *posB = GetPosition(&ecs.position, entityB);
            if (!posB) continue;

            // Mask filtering
            bool canA_hit_B = (colA->collision_layer & colB->collision_mask) != 0;
            bool canB_hit_A = (colB->collision_layer & colA->collision_mask) != 0;
            
            if (!canA_hit_B && !canB_hit_A) continue; 

            // Create cute_c2 shapes
            c2Manifold manifold;
            manifold.count = 0;

            // Entity A Shape
            c2AABB boxA;
            c2Circle circleA;
            if (colA->type == COLLISION_REC) {
                boxA.min = (c2v){ posA->x + colA->offsets.x, posA->y + colA->offsets.y };
                boxA.max = (c2v){ boxA.min.x + colA->size.x, boxA.min.y + colA->size.y };
            } else if (colA->type == COLLISION_CIRCLE) {
                circleA.p = (c2v){
                    posA->x + colA->offsets.x + colA->size.x / 2.0f,
                    posA->y + colA->offsets.y + colA->size.x / 2.0f 
                };
                circleA.r = colA->size.x / 2.0f;
            }

            // Entity B Shape
            c2AABB boxB;
            c2Circle circleB;
            if (colB->type == COLLISION_REC) {
                boxB.min = (c2v){ posB->x + colB->offsets.x, posB->y + colB->offsets.y };
                boxB.max = (c2v){ boxB.min.x + colB->size.x, boxB.min.y + colB->size.y };
            } else if (colB->type == COLLISION_CIRCLE) {
                circleB.p = (c2v){
                    posB->x + colB->offsets.x + colB->size.x / 2.0f,
                    posB->y + colB->offsets.y + colB->size.x / 2.0f
                };
                circleB.r = colB->size.x / 2.0f;
            }

            // Calculate Manifold with cute_c2
            if (colA->type == COLLISION_REC && colB->type == COLLISION_REC) {
                c2AABBtoAABBManifold(boxA, boxB, &manifold);
            } else if (colA->type == COLLISION_CIRCLE && colB->type == COLLISION_CIRCLE) {
                c2CircletoCircleManifold(circleA, circleB, &manifold);
            } else if (colA->type == COLLISION_REC && colB->type == COLLISION_CIRCLE) {
                c2CircletoAABBManifold(circleB, boxA, &manifold);
                manifold.n.x = -manifold.n.x;
                manifold.n.y = -manifold.n.y;
            } else if (colA->type == COLLISION_CIRCLE && colB->type == COLLISION_REC) {
                c2CircletoAABBManifold(circleA, boxB, &manifold);
            }

            // Detection & Resolution
            if (manifold.count > 0) {
                // Save the collision events with the "real" ids
                if (ecs.collision_event_count < MAX_COLLISION_EVENTS) {
                    ecs.frame_collisions[ecs.collision_event_count].entity_a = entityA;
                    ecs.frame_collisions[ecs.collision_event_count].entity_b = entityB;
                    ecs.collision_event_count++;
                }

                if (colA->is_trigger || colB->is_trigger) continue;

                float depth = manifold.depths[0];
                c2v n = manifold.n;

                bool staticA = colA->is_static;
                bool staticB = colB->is_static;

                // Lookup στο Velocity (physical resolution)
                VelocityComponent *velA = GetVelocity(&ecs.velocity, entityA);
                VelocityComponent *velB = GetVelocity(&ecs.velocity, entityB);

                // A Dynamic, B Static
                if (!staticA && staticB) {
                    posA->x -= n.x * depth;
                    posA->y -= n.y * depth;
                
                    if (velA) {
                        if (n.x != 0.0f && (velA->vx * n.x > 0)) velA->vx = 0.0f;
                        if (n.y != 0.0f && (velA->vy * n.y > 0)) velA->vy = 0.0f;
                    }
                }
                // A Static, B Dynamic
                else if (staticA && !staticB) {
                    posB->x += n.x * depth;
                    posB->y += n.y * depth;

                    if (velB) {
                        if (n.x != 0.0f && (velB->vx * n.x < 0)) velB->vx = 0.0f;
                        if (n.y != 0.0f && (velB->vy * n.y < 0)) velB->vy = 0.0f;
                    }
                }
                // A Dynamic, B Dynamic (50/50 pushback)
                else if (!staticA && !staticB) {
                    float halfDepth = depth * 0.5f;
                    posA->x -= n.x * halfDepth;
                    posA->y -= n.y * halfDepth;

                    posB->x += n.x * halfDepth;
                    posB->y += n.y * halfDepth;
                }
            }
        }
    }
}

void ECS_DebugRenderSystem(Camera2D camera)
{
    BeginMode2D(camera);

    for (uint32_t i = 0; i < ecs.collision.count; i++)
    {
        uint32_t entity_id = ecs.collision.packed_to_entity[i];
        CollisionComponent *col = &ecs.collision.data[i];
        PositionComponent *pos = GetPosition(&ecs.position, entity_id);

        if (!pos) continue;

        // Here is the selection for the color. Red for triggers green for static
        Color debug_color = col->is_trigger ? GREEN : RED;

        if (col->type == COLLISION_REC)
        {
            // calc the rec with teh offsets
            Rectangle rec = {
                .x = pos->x + col->offsets.x,
                .y = pos->y + col->offsets.y,
                .width = col->size.x,
                .height = col->size.y
            };
            
            // Draw the collision rec 
            DrawRectangleLinesEx(rec, 1.0f, debug_color);
        }
        else if (col->type == COLLISION_CIRCLE)
        {
            // caclulate the center of the crircle (like cute_c2)
            Vector2 center = {
                .x = pos->x + col->offsets.x + col->size.x / 2.0f,
                .y = pos->y + col->offsets.y + col->size.x / 2.0f
            };
            float radius = col->size.x / 2.0f;

            // Draw tge circle 
            DrawCircleLinesV(center, radius, debug_color);
        }
    }

    EndMode2D();
}