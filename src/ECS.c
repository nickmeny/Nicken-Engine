
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
#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"

//Here i loop the entities and i use the cute_c2 lib to detect collision. Maybe it can be otpimised but in the futer
//TODO: OPTIMIZE THE CODE
void ECS_CollisionSystem(float dt) {
    (void)dt; // Unused parameter
    
    //In every frame we "throw" the previws collison and strat again
    ecs.collision_event_count = 0;

    for (int i = 0; i < MAX_ENTITIES; i++) { //for every entity
        uint32_t reqA = COMPOMENT_POSITION | COMPONENT_COLLISION; //the mask for the COLLISION DETECTION
        if ((ecs.entinty_bitmask[i] & reqA) != reqA) continue; //skip if the entity dont have that specific bitmask

        for (int j = i + 1; j < MAX_ENTITIES; j++) {// for the the next entitys
            uint32_t reqB = COMPOMENT_POSITION | COMPONENT_COLLISION;
            if ((ecs.entinty_bitmask[j] & reqB) != reqB) continue;

            //Mask filtering
            //here is one more optimizetion and one more feature for the collisions, with collisions layers and masks
            bool canA_hit_B = (ecs.collision[i].collision_layer & ecs.collision[j].collision_mask) != 0;
            bool canB_hit_A = (ecs.collision[j].collision_layer & ecs.collision[i].collision_mask) != 0;
            
            if (!canA_hit_B && !canB_hit_A) continue; 

            // Create the cute_c2 shapes ( AABB,Circle)
            c2Manifold manifold;
            manifold.count = 0;

            // Entity A Shape
            c2AABB boxA;
            c2Circle circleA;
            //here I init the different collisions for each enityt
            switch (ecs.collision[i].type)
            {
            case COLLISION_REC:
                boxA.min = (c2v){ 
                    ecs.position[i].x + ecs.collision[i].offsets.x, 
                    ecs.position[i].y + ecs.collision[i].offsets.y
                };
                boxA.max = (c2v){
                    boxA.min.x + ecs.collision[i].size.x, 
                    boxA.min.y + ecs.collision[i].size.y 
                };
                break;
            case COLLISION_CICLE:
                circleA.p = (c2v){
                    ecs.position[i].x + ecs.collision[i].offsets.x + ecs.collision[i].size.x / 2.0f,
                    ecs.position[i].y + ecs.collision[i].offsets.y + ecs.collision[i].size.x / 2.0f 
                };
                circleA.r = ecs.collision[i].size.x / 2.0f;
            default:
                break;
            }

            // Entity B Shape
            c2AABB boxB;
            c2Circle circleB;
            switch (ecs.collision[j].type)
            {
            case COLLISION_REC:
                boxB.min = (c2v){
                    ecs.position[j].x + ecs.collision[j].offsets.x,
                    ecs.position[j].y + ecs.collision[j].offsets.y
                };
                boxB.max = (c2v){
                    boxB.min.x + ecs.collision[j].size.x,
                    boxB.min.y + ecs.collision[j].size.y
                };
                break;
            case COLLISION_CICLE:
                circleB.p = (c2v){
                    ecs.position[j].x + ecs.collision[j].offsets.x + ecs.collision[j].size.x / 2.0f,
                    ecs.position[j].y + ecs.collision[j].offsets.y + ecs.collision[j].size.x / 2.0f
                };
                circleB.r = ecs.collision[j].size.x / 2.0f;
                break;
            default:
                break;
            }

            // Calculate Mainfold with cute_c2
            if (ecs.collision[i].type == COLLISION_REC && ecs.collision[j].type == COLLISION_REC) {
                c2AABBtoAABBManifold(boxA, boxB, &manifold);
            } else if (ecs.collision[i].type == COLLISION_CICLE && ecs.collision[j].type == COLLISION_CICLE) {
                c2CircletoCircleManifold(circleA, circleB, &manifold);
            } else if (ecs.collision[i].type == COLLISION_REC && ecs.collision[j].type == COLLISION_CICLE) {
                c2CircletoAABBManifold(circleB, boxA, &manifold);
                // Reverse the normal because the pos of the parametres are changed
                manifold.n.x = -manifold.n.x;
                manifold.n.y = -manifold.n.y;
            } else if (ecs.collision[i].type == COLLISION_CICLE && ecs.collision[j].type == COLLISION_REC) {
                c2CircletoAABBManifold(circleA, boxB, &manifold);
            }

            // Detection & Resolution
            if (manifold.count > 0) {
                //Here i save the event so the lua can access it
                if (ecs.collision_event_count < MAX_COLLISION_EVENTS) {
                    ecs.frame_collisions[ecs.collision_event_count].entity_a = i;
                    ecs.frame_collisions[ecs.collision_event_count].entity_b = j;
                    ecs.collision_event_count++;
                }

                // Προσπερνάμε το physical pushback αν κάποιο είναι trigger
                if (ecs.collision[i].is_trigger || ecs.collision[j].is_trigger) continue;

                //the number of pixels the overlap dows ( for example 3 px) and the vector of the direction of the collision
                float depth = manifold.depths[0];
                c2v n = manifold.n;

                bool staticA = ecs.collision[i].is_static;
                bool staticB = ecs.collision[j].is_static;

                //  if A is dynamyc and b is static
                if (!staticA && staticB) {
                    ecs.position[i].x -= n.x * depth;
                    ecs.position[i].y -= n.y * depth;
                
                    //if the collision is for down (for example the floor) the normal vector is point up, n.y<0
                    if (n.y < 0.0f && ecs.velocity[i].vy > 0.0f) {
                        ecs.velocity[i].vy = 0.0f;  //stop moving
                    }
                    // Χτύπημα σε ΤΑΒΑΝΙ από κάτω (Normal δείχνει προς τα ΚΑΤΩ, δηλαδή n.y > 0)
                    //if the a is collide ith b from under ( for example cell), the n vector is point down ( n.y>0)
                    else if (n.y > 0.0f && ecs.velocity[i].vy < 0.0f) {
                        ecs.velocity[i].vy = 0.0f; //stop moving to start the dwnfall
                    }
                }
                //A = Static, B = Dynamic
                else if (staticA && !staticB) {
                    ecs.position[j].x += n.x * depth;
                    ecs.position[j].y += n.y * depth;

                    if (n.x != 0.0f && (ecs.velocity[j].vx * n.x < 0)) ecs.velocity[j].vx = 0.0f;
                    if (n.y != 0.0f && (ecs.velocity[j].vy * n.y < 0)) ecs.velocity[j].vy = 0.0f;
                }
                // The A and B dynamic , 50/50 pushback
                else if (!staticA && !staticB) {
                    float halfDepth = depth * 0.5f;
                    ecs.position[i].x -= n.x * halfDepth;
                    ecs.position[i].y -= n.y * halfDepth;

                    ecs.position[j].x += n.x * halfDepth;
                    ecs.position[j].y += n.y * halfDepth;
                }
            }
        }
    }
}