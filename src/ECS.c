
#include <stdio.h>
#include <inttypes.h>
#include "ECS.h"
#include "rlgl.h"
#include "raymath.h"
#include "math.h"
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
    uint64_t key; //32 bits layer, 31-bit -> 1-bit tex id [1-bit Type(0 sprite 1 mesh)]
    uint32_t entity_id;
    float w;
    float h;
}RenderCommand;

static RenderCommand render_commands[MAX_ENTITIES];

//Function fot qsort
int CompareRenderCommands(const void *a, const void *b) {
    uint64_t keyA = ((RenderCommand*)a)->key;
    uint64_t keyB = ((RenderCommand*)b)->key;
    if (keyA < keyB) return -1;
    if (keyA > keyB) return 1;
    return 0;
}

//To clean the ECS
void InitECS(void)
{
    // Cleare all the ecs;
    memset(&ecs, 0, sizeof(ECS));

    // Here Initilize all the spare pools
    InitPositionPool(&ecs.position);
    InitCollisionPool(&ecs.collision);
    InitVelocityPool(&ecs.velocity);
    InitSpritePool(&ecs.sprite);
    InitMeshPool(&ecs.mesh);

    // Put all the ids in free list
    for (int i = 0; i < MAX_ENTITIES; i++) {
        ecs.free_list_ids[i] = i;
    }
    ecs.free_list_count = MAX_ENTITIES; //start from the end
}

//THis method allows the create entity to be in O(1). Becasue the system add the free id in the end and it dows zero swifts

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

    // Add the id in the free list
    if (ecs.free_list_count < MAX_ENTITIES) {
        ecs.free_list_ids[ecs.free_list_count] = entity_id;
        ecs.free_list_count++;
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
    uint32_t command_count = 0;
    //Here i "fetch" all the sprites components. Because i have the Pools i can iritate only the enetitys the have the sprite and not just "continue" in 
    //loop, sth that creates a lot of cache misses
    for (uint32_t i = 0; i < ecs.sprite.count; i++)
    {   
        //here i get the actuall id of the entity
        uint32_t entity_id = ecs.sprite.packed_to_entity[i];
        SpriteComponent *sprite = &ecs.sprite.data[i]; //here i get the compoment
        if (sprite->texture_id == 0) //if a texture is not load just skip
            continue;
        uint64_t z_key = ((uint64_t)sprite->render_layer) << 32; //Here i do the "trick" to save space i use in the struct a uint64_t bit so i can use evrey bit as i want
        //The first 32 bits are the layer, the depth of the sprite[from bit 63 to bit 32]
        uint64_t tex_key = ((uint64_t)sprite->texture_id) << 1; //here is the texture id, from [31->1]
        render_commands[command_count].key = z_key | tex_key | 0; // There I do the combination of the bits. I use the OR and the 0 is the last bit that tells the system is a sprite(0) or a mesh(1) because i wamnt sprite i put 0
        render_commands[command_count].entity_id = entity_id; //put the id
        render_commands[command_count].w = (sprite->width > 0) ? sprite->width : 64.0f;
        render_commands[command_count].h = (sprite->height > 0) ? sprite->height : 64.0f;
        command_count++; //plus by one the counter ( this counter is for the batch commands)
    }
    //Here i do the same as the sprites but for the Meshes
    for (uint32_t i = 0; i < ecs.mesh.count; i++)
    {
        MeshComponent *mesh = &ecs.mesh.data[i];

        uint64_t layer_key = ((uint64_t)mesh->render_layer) << 32;
        uint64_t tex_key = 0; // 0 Texture ID for meshes

        render_commands[command_count].key = layer_key | tex_key | 1; // Bit 0 = 1 (Mesh)
        render_commands[command_count].entity_id = ecs.mesh.packed_to_entity[i];
        command_count++;
    }
    //If no render commands return from the func
    if (command_count == 0)
        return;
    //quick sort the commands with the keys
    qsort(render_commands, command_count, sizeof(RenderCommand), CompareRenderCommands);

    //Here is starting the actuall rendering
    BeginMode2D(camera);
    BeginShaderMode(circleShader);
    uint32_t current_tex = (uint32_t)-1; // The current texture is -1
    bool in_batch = false; // is a switch to know when the gpu has to flash the shader 
    //for all the render commands
    for (uint32_t i = 0; i < command_count; i++)
    {
        uint32_t entity_id = render_commands[i].entity_id; //get the id
        uint64_t key = render_commands[i].key; //get the key
        bool is_mesh = (key & 1); //If is a mesh means the last beat is 1. e.x. 1011(mesh) & 0001 = 0001(true) 1010(sprite)&0001 =0000(false)

        uint32_t tex_id = is_mesh ? 0 : (uint32_t)((key >> 1) & 0x7FFFFFFF); //Here is one more trick i do. The number 0x7FFFFFFF it has 31 ones. The first
        //think i do is to swift the number 1 bit right to throw out the mesh/sprite bit. after that i have a number like
        // 0[32bits layer][31 bits id]. I apply the mask of 0x7FFFFFFF so i "cancel" the bits after the 31st bit.

        PositionComponent *pos = GetPosition(&ecs.position, entity_id); //get the position component
        if (!pos) //if there aren't any just keep
            continue;
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
        //Position
        float x = pos->x;
        float y = pos->y;

        //For Spirtes 
        if (!is_mesh)
        {
            // SPRITE: Standard UVs 
            SpriteComponent *sprite = GetSprite(&ecs.sprite, entity_id);
            rlColor4ub(255, 255, 255, 255);
            float w = (sprite->width > 0) ? sprite->width : 64.0f;
            float h = (sprite->height > 0) ? sprite->height : 64.0f;

            rlTexCoord2f(0.0f, 0.0f);
            rlVertex2f(x, y);
            rlTexCoord2f(0.0f, 1.0f);
            rlVertex2f(x, y + h);
            rlTexCoord2f(1.0f, 1.0f);
            rlVertex2f(x + w, y + h);
            rlTexCoord2f(1.0f, 0.0f);
            rlVertex2f(x + w, y);
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