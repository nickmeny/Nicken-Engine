#include "lua_bindings.h"
#include "ECS.h"
#include <string.h>
#include <stdlib.h>
//====================================================================================
//                                    Engine Flags & State
//====================================================================================
static bool g_is_window_init = false;
static Map g_map = NULL;
static Map g_texture_cache = NULL;

static int compare_strings(Pointer a, Pointer b) {
    return strcmp((const char*)a, (const char*)b);
}
static void destroy_key_string(Pointer key) {
    free((void*)key);
}
static void destroy_value_texture(Pointer value) {
    Texture2D* tex = (Texture2D*)value;
    if (tex) {
        free(tex);           
    }
}

Texture2D GetOrLoadTexture(const char* path) {
    if (g_texture_cache == NULL) {
        g_texture_cache = map_create(compare_strings, destroy_key_string, destroy_value_texture);
        map_set_hash_function(g_texture_cache, hash_string);
    }

    Texture2D* cached = (Texture2D*)map_find(g_texture_cache, (Pointer)path);
    if (cached != NULL) {
        return *cached;
    }

    Texture2D new_tex = LoadTexture(path);
    
    Texture2D* store_tex = malloc(sizeof(Texture2D));
    *store_tex = new_tex;

    char* permanent_path = strdup(path);
    map_insert(g_texture_cache, (Pointer)permanent_path, (Pointer)store_tex);

    printf("[ASSET MANAGER] Loaded NEW texture: %s \n", path);

    return new_tex;
}
void UnloadTextureCache(void) {
    if (g_texture_cache != NULL) {
        map_destroy(g_texture_cache);
        g_texture_cache = NULL;
    }
}

bool IsWindowInitialized(void) {
    return g_is_window_init;
}

/*
===================================================================================================
------------------------------------PARSERS FOR COMPOMENTS---------------------------------------
===================================================================================================
*/

static void position_parser(lua_State * L, int id)
{
    /*
    I put the absulote stack pointer in a variable so not to worry what position the table will be after the getfielad ( absolute pointer not move. if the 
    table was in stack index 2 it will be in 2 not matter how match thinks i push in).
    */
    int lua_table = lua_gettop(L);

    lua_getfield(L,lua_table,"x");
    lua_getfield(L,lua_table,"y");
    
    PositionComponent pos= {
        .x=(float)luaL_optnumber(L,-2,0.0), //Because the getgilead push the items down the first value is the last one so the y, but i wantthe x, that is in the second level
        .y =(float)luaL_optnumber(L,-1,0.0) 
    };
    AddPosition(&ecs.position,id,pos);
    lua_pop(L,2); //I pop the stack for x and y and i return it as it was ( first think the table) so the lua can clean it with garbage collector
}

//The same as the position parser
static void velocity_parser(lua_State *L, int id)
{
    int table_idx = lua_gettop(L); 

    lua_getfield(L, table_idx, "vx");
    lua_getfield(L, table_idx, "vy");

    VelocityComponent velocity = {
        .vx = (float)luaL_optnumber(L, -2, 0.0),
        .vy = (float)luaL_optnumber(L, -1, 0.0)
    };
    AddVelocity(&ecs.velocity, id, velocity);
    lua_pop(L, 2);
}

static Color parse_color(lua_State * L,int index )
{
    Color color = WHITE;
    int type = lua_type(L,index);
    if (type == LUA_TSTRING)
    {
        const char *color_str = lua_tostring(L, index);
        MATCH_COLOR(color_str, "red",    RED);
        MATCH_COLOR(color_str, "green",  GREEN);
        MATCH_COLOR(color_str, "blue",   BLUE);
        MATCH_COLOR(color_str, "black",  BLACK);
        MATCH_COLOR(color_str, "gray",   GRAY);
        MATCH_COLOR(color_str, "yellow", YELLOW);
        MATCH_COLOR(color_str, "white",  WHITE);
    }
    else if(type==LUA_TTABLE)
    {
        int color_table_idx = lua_absindex(L, index);

        lua_getfield(L, color_table_idx, "r");
        lua_getfield(L, color_table_idx, "g");
        lua_getfield(L, color_table_idx, "b");
        lua_getfield(L, color_table_idx, "a");

        color.r = (unsigned char)luaL_optinteger(L, -4, 255);
        color.g = (unsigned char)luaL_optinteger(L, -3, 255);
        color.b = (unsigned char)luaL_optinteger(L, -2, 255);
        color.a = (unsigned char)luaL_optinteger(L, -1, 255);

        lua_pop(L, 4);
    }
    return color;
}

static void mesh_parser(lua_State * L,int id)
{
    int table_idx = lua_gettop(L);
    MeshComponent mesh = {0};
    lua_getfield(L,table_idx,"type");
    const char* type = luaL_optstring(L,-1,"rec");
    if(strcmp(type,"rec")==0)
    {
      mesh.type = MESH_RECTANGLE;
    }else if (strcmp(type,"circle")==0)
    {
        mesh.type = MESH_CIRCLE;
    }else
    {
        mesh.type = MESH_NONE;
    }

    lua_pop(L,1);
    lua_getfield(L,table_idx,"size");

    if(!lua_istable(L,-1)) 
    {
        mesh.size = (Vector2){10,10};
        lua_pop(L,1);
    }else{
        int size_table_index = lua_gettop(L);
        lua_getfield(L,size_table_index,"x");
        lua_getfield(L,size_table_index,"y");
        mesh.size = (Vector2){
            .x = (int)luaL_optinteger(L,-2,1),
            .y = (int)luaL_optinteger(L,-1,1)
        };
        lua_pop(L,3);
    }
    lua_getfield(L,table_idx,"color");
    mesh.color = parse_color(L,-1);
    lua_pop(L,1);
    lua_getfield(L, table_idx, "layer");
    mesh.render_layer = (int)luaL_optinteger(L, -1, 0);
    lua_pop(L, 1);
    AddMesh(&ecs.mesh,id,mesh);
}


static void animation_parser(lua_State *L, int id)
{
    int table_index = lua_gettop(L);
    AnimationComponent anim = {0};
    anim.current_clip = -1;

    lua_getfield(L, table_index, "frame_width");
    anim.frame_width = (int)luaL_optinteger(L, -1, 32);
    lua_pop(L, 1);

    lua_getfield(L, table_index, "frame_height");
    anim.frame_height = (int)luaL_optinteger(L, -1, 32);
    lua_pop(L, 1);

    lua_getfield(L, table_index, "animations");
    if (lua_istable(L, -1)) {
        int anims_table = lua_gettop(L);
        lua_pushnil(L);
        
        while (lua_next(L, anims_table) != 0)
        {
            if (anim.clip_count < MAX_ANIMATIONS_PER_ENTITY)
            {
                AnimationClip *clip = &anim.clips[anim.clip_count];
                
                // copy the name and put the \0 byte in the end
                const char* name = lua_tostring(L, -2);
                if (name) {
                    strncpy(clip->name, name, sizeof(clip->name) - 1);
                    clip->name[sizeof(clip->name) - 1] = '\0';
                }

                lua_getfield(L, -1, "row");
                clip->row = (int)luaL_optinteger(L, -1, 0);
                lua_pop(L, 1);

                lua_getfield(L, -1, "frames");
                clip->frame_count = (int)luaL_optinteger(L, -1, 1);
                lua_pop(L, 1);

                lua_getfield(L, -1, "speed");
                clip->speed = (float)luaL_optnumber(L, -1, 10.0);
                lua_pop(L, 1);

                lua_getfield(L, -1, "loop");
                clip->loop = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : true;
                lua_pop(L, 1);

                anim.clip_count++;
            }
            lua_pop(L, 1); // Pop value, keep the key for the next iretetion
        }
    }
    lua_pop(L, 1); // Pop 'animations' table

    // default animation
    lua_getfield(L, table_index, "default_animation");
    const char* def_anim = luaL_optstring(L, -1, NULL);
    if (def_anim) {
        PlayAnimationByName(&anim, def_anim);
    } else if (anim.clip_count > 0) {
        anim.current_clip = 0; // Fallback 
    }
    lua_pop(L, 1);

    AddAnimation(&ecs.animation, id, anim);
}
static void collision_parser(lua_State * L,int id)
{
    int table_idx = lua_gettop(L);
    CollisionComponent collision = {0};

    lua_getfield(L, table_idx, "type");
    const char* type_str = luaL_optstring(L, -1, "rec");
    if (strcmp(type_str, "circle") == 0) {
        collision.type = COLLISION_CIRCLE;
    } else {
        collision.type = COLLISION_REC;
    }
    lua_pop(L, 1);

    lua_getfield(L, table_idx, "size");
    if (!lua_istable(L, -1)) {
        collision.size = (Vector2){10.0f, 10.0f}; // Default size
        lua_pop(L,1);
    } else {
        int size_table_index = lua_gettop(L);
        lua_getfield(L,size_table_index,"x");
        lua_getfield(L,size_table_index,"y");
        collision.size.x = (float)luaL_optnumber(L, -2, 10.0f);
        collision.size.y = (float)luaL_optnumber(L, -1, 10.0f);
        lua_pop(L, 3); // pop x, y
    }
    lua_getfield(L, table_idx, "offset");
    if (!lua_istable(L, -1)) {
        collision.offsets = (Vector2){0.0f, 0.0f}; // Default size
        lua_pop(L,1);
    }else{
        int offset_table_index = lua_gettop(L);
        lua_getfield(L,offset_table_index,"x");
        lua_getfield(L,offset_table_index,"y");
        collision.offsets.x = (float)luaL_optnumber(L, -2, 0.0f);
        collision.offsets.y = (float)luaL_optnumber(L, -1, 0.0f);
        lua_pop(L, 3); // pop x, y
    } 
    lua_getfield(L, table_idx, "layer");
    collision.collision_layer = (uint32_t)luaL_optinteger(L, -1, 1);
    lua_pop(L, 1);

    lua_getfield(L, table_idx, "is_static");
    collision.is_static = (uint32_t)lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, table_idx, "mask");
    collision.collision_mask = (uint32_t)luaL_optinteger(L, -1, 1);
    lua_pop(L, 1);
    AddCollision(&ecs.collision,id,collision);
}

static void texture_parser(lua_State *L, int id)
{
    int table_index = lua_gettop(L);
    SpriteComponent sprite = {0};
    
    lua_getfield(L, table_index, "path");
    const char *path = luaL_optstring(L, -1, NULL);
    if (path == NULL) luaL_error(L, "[ERROR] Must specify a path for the texture");
    
    Texture2D tex = GetOrLoadTexture(path);
    sprite.texture_id = tex.id;
    sprite.texture_w = (float)tex.width;
    sprite.texture_h = (float)tex.height;
    lua_pop(L, 1);

    lua_getfield(L, table_index, "layer");
    sprite.render_layer = (int)luaL_optinteger(L, -1, 0);
    lua_pop(L, 1);

    // Parsing Size (Υποστήριξη x/y & w/h)
    lua_getfield(L, table_index, "size");
    if (lua_istable(L, -1))
    {
        int size_idx = lua_gettop(L);
        
        lua_getfield(L, size_idx, "x");
        float sx = (float)luaL_optnumber(L, -1, -1.0f);
        lua_pop(L, 1);

        lua_getfield(L, size_idx, "w");
        float sw = (float)luaL_optnumber(L, -1, -1.0f);
        lua_pop(L, 1);

        lua_getfield(L, size_idx, "y");
        float sy = (float)luaL_optnumber(L, -1, -1.0f);
        lua_pop(L, 1);

        lua_getfield(L, size_idx, "h");
        float sh = (float)luaL_optnumber(L, -1, -1.0f);
        lua_pop(L, 1);

        sprite.width = (sx >= 0.0f) ? sx : ((sw >= 0.0f) ? sw : sprite.texture_w);
        sprite.height = (sy >= 0.0f) ? sy : ((sh >= 0.0f) ? sh : sprite.texture_h);
    } 
    else 
    {
        // Fallback στο Frame size if there a re already animation 
        AnimationComponent *anim = GetAnimation(&ecs.animation, id);
        if (anim && anim->frame_width > 0) {
            sprite.width = (float)anim->frame_width;
            sprite.height = (float)anim->frame_height;
        } else {
            sprite.width = sprite.texture_w;
            sprite.height = sprite.texture_h;
        }
    }
    lua_pop(L, 1); // pop 'size'

    // Parsing Frame/Offset
    lua_getfield(L, table_index, "frame");
    if (lua_istable(L, -1))
    {
        int frame_idx = lua_gettop(L);
        lua_getfield(L, frame_idx, "x");
        lua_getfield(L, frame_idx, "y");
        sprite.x = (float)luaL_optnumber(L, -2, 0.0f);
        sprite.y = (float)luaL_optnumber(L, -1, 0.0f);
        lua_pop(L, 2);
    } else {
        lua_getfield(L, table_index, "src_x");
        sprite.x = (float)luaL_optnumber(L, -1, 0.0f);
        lua_pop(L, 1);

        lua_getfield(L, table_index, "src_y");
        sprite.y = (float)luaL_optnumber(L, -1, 0.0f);
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop 'frame'

    // Flips
    lua_getfield(L, table_index, "flip_x");
    sprite.flip_x = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, table_index, "flip_y");
    sprite.flip_y = lua_toboolean(L, -1);
    lua_pop(L, 1);

    AddSprite(&ecs.sprite, id, sprite);
}

//=================================================================================
//                      Parser Registry Table
//==================================================================================


//A generic form for the Compoment parser. Becaue i want to skip to write manually  hunderds id i crete a generic form for the parsers
// The key is the keyword the user give in lua table, for exampe {position={x=100,y=300}}
// the mask_bit is the COMPOMENT_NAME that will help me later in physics and rendering, it is like a look up table
//the  last is the parser function that take the lua file and a id 
typedef struct {
    const char * key;
    void (*parser)(lua_State*L,int id); 
}CompomentParser;


//Because of the generic struct now i can create a table of structs and skip the "spagety code"
static const CompomentParser COMPOMENT_PARSERS[] ={
    {"position",position_parser},
    {"velocity",velocity_parser},
    {"mesh",mesh_parser},
    {"collision",collision_parser},
    {"texture",texture_parser},
    {"animation",animation_parser},
    
};

//A Counter to know how many items i have in the table above
static const size_t PARSER_COUNT = sizeof(COMPOMENT_PARSERS)/sizeof(COMPOMENT_PARSERS[0]);

/*
================================================================================================
------------------------------------Lua Banding Functions---------------------------------------
================================================================================================
*/

int C_CreateEntity(lua_State *L)
{
    //Get the next free id 
    int id = CreateEntity();
    //If i have no free IDs it means the engine is max out
    if(id==-1){
        luaL_error(L,"ECS Error:  Reached MAX_ENTITIES capacity");
        return 0;
    }
    //if the func has no table means the user want a empty entity
    if(lua_gettop(L)==0 || !lua_istable(L,1))
    {
        lua_pushinteger(L,id);
        return 1;
    }
    //Because i have the option for prefabs, I have a system for override a prefab
    //If the arguments are more than 2 and the second argument is a table
    if(lua_gettop(L) >= 2 && lua_istable(L, 2)) 
    {
        //I initialize the key for the lua table to be nil to start the loop
        lua_pushnil(L); 
        //This take the current key from the top of the stack, it pops it and put the next pair of keys-valeus in its position.
        //If there is a item, returns true and put in stack the key in -2 and the value in -1. If is in the end of the table is return false and do nothing
        while (lua_next(L, 2) != 0)
        {
            lua_pushvalue(L, -2); // push key
            lua_pushvalue(L, -2); // push value            
            lua_settable(L, 1); 
            lua_pop(L, 1); 
        }
    }
    //I look the table of parsers
    for (size_t i = 0; i < PARSER_COUNT; i++) {
        lua_getfield(L, 1, COMPOMENT_PARSERS[i].key);
        if (!lua_isnil(L, -1)) {
            COMPOMENT_PARSERS[i].parser(L, id);
        }
        lua_pop(L, 1);
    }

    lua_pushinteger(L, id);
    return 1;
}


int C_Window_Initialize(lua_State * L)
{
    int width = luaL_checkinteger(L,1);
    int height = luaL_checkinteger(L,2);
    const char * title = luaL_checkstring(L,3);
    InitWindow(width,height,title);
    SetTargetFPS(60);
    g_is_window_init = true;
    return 0;
}

void RegisterMapAction(Map map)
{
    g_map=map;
}

int C_is_Action_Down(lua_State * L)
{
    const char * action = luaL_checkstring(L,1);
    KeyboardKey* key_ptr = (KeyboardKey*)map_find(g_map, (Pointer)action);
    bool press = false;
    if(key_ptr!=NULL)
    {
        press = IsKeyDown(*key_ptr);
    }
    lua_pushboolean(L,press);
    return 1;
}
int C_SetPosition(lua_State *L) {
    uint32_t id = (uint32_t)luaL_checkinteger(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    PositionComponent *pos = GetPosition(&ecs.position, id);
    if (pos) {
        pos->x = x;
        pos->y = y;
    } else {
        AddPosition(&ecs.position, id, (PositionComponent){x, y});
    }
    return 0;
}

int C_SetVelocity(lua_State *L) {
    uint32_t id = (uint32_t)luaL_checkinteger(L, 1);
    float vx = (float)luaL_checknumber(L, 2);
    float vy = (float)luaL_checknumber(L, 3);

    VelocityComponent *vel = GetVelocity(&ecs.velocity, id);
    if (vel) {
        vel->vx = vx;
        vel->vy = vy;
    } else {
        AddVelocity(&ecs.velocity, id, (VelocityComponent){vx, vy});
    }
    return 0;
}

int C_IsCollide(lua_State *L) {
    int id = (int)luaL_checkinteger(L, 1);
    int id2 = (int)luaL_checkinteger(L, 2);
    
    for (int i = 0; i < ecs.collision_event_count; i++) {
        if ((ecs.frame_collisions[i].entity_a == id && ecs.frame_collisions[i].entity_b == id2) ||
            (ecs.frame_collisions[i].entity_a == id2 && ecs.frame_collisions[i].entity_b == id)) {
            lua_pushboolean(L, 1);
            return 1;
        }
    }
    lua_pushboolean(L, 0);
    return 1;
}
int C_GetVelocity(lua_State *L) {
    uint32_t id = (uint32_t)luaL_checkinteger(L, 1);
    VelocityComponent *vel = GetVelocity(&ecs.velocity, id);
    if (vel) {
        lua_pushnumber(L, vel->vx);
        lua_pushnumber(L, vel->vy);
        return 2;
    }
    lua_pushnumber(L, 0);
    lua_pushnumber(L, 0);
    return 2;
}

int C_GetPosition(lua_State *L) {
    uint32_t id = (uint32_t)luaL_checkinteger(L, 1);
    PositionComponent *pos = GetPosition(&ecs.position, id);
    if (pos) {
        lua_pushnumber(L, pos->x);
        lua_pushnumber(L, pos->y);
        return 2;
    }
    lua_pushnumber(L, 0);
    lua_pushnumber(L, 0);
    return 2;
}

int C_SetFlip(lua_State * L)
{
    uint32_t id = (uint32_t)luaL_checkinteger(L, 1);
    bool flip_x = lua_toboolean(L,2);
    bool flip_y = lua_toboolean(L,3);
    SpriteComponent *sprite = GetSprite(&ecs.sprite,id);
    if(sprite)
    {
        sprite->flip_x = flip_x;
        sprite->flip_y = flip_y;
    }
    return 0;
}
int C_PlayAnimation(lua_State *L) {
    uint32_t id = (uint32_t)luaL_checkinteger(L, 1);
    const char *anim_name = luaL_checkstring(L, 2);

    AnimationComponent *anim = GetAnimation(&ecs.animation, id);
    if (anim) {
        PlayAnimationByName(anim, anim_name);
    }
    return 0;
}

int C_DestroyEntity(lua_State* L)
{
    uint32_t id = (uint32_t)luaL_checkinteger(L,1);
    DestroyEntity(id);
    return 0;
    
}

//========================================================================
//                          Engine Module Registration
//========================================================================

static const struct luaL_Reg engine_funcs[] = {
    {"window_init", C_Window_Initialize},
    {"create_entity", C_CreateEntity},
    {"IsActionPressed",C_is_Action_Down},
    {"set_position", C_SetPosition},
    {"set_velocity",C_SetVelocity},
    {"is_collide",C_IsCollide},
    {"get_velocity",C_GetVelocity},
    {"get_position",C_GetPosition},
    {"set_flip",C_SetFlip},
    {"play_animation",   C_PlayAnimation},
    {"destroy_entity",C_DestroyEntity},
    {NULL, NULL}
};


void RegisterEngineFunctions(lua_State *L) {
    luaL_newlib(L, engine_funcs);
    lua_setglobal(L, "Engine");
}