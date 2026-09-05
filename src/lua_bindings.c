#include "lua_bindings.h"
#include "ECS.h"
#include <string.h>
//====================================================================================
//                                    Engine Flags & State
//====================================================================================
static bool g_is_window_init = false;
static Map g_map = NULL;


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
    
    ecs.position[id] = (PositionCompoment){
        .x=(float)luaL_optnumber(L,-2,0.0), //Because the getgilead push the items down the first value is the last one so the y, but i wantthe x, that is in the second level
        .y =(float)luaL_optnumber(L,-1,0.0) 
    };
    lua_pop(L,2); //I pop the stack for x and y and i return it as it was ( first think the table) so the lua can clean it with garbage collector
}

//The same as the position parser
static void velocity_parser(lua_State *L,int id)
{
    lua_getfield(L,-1,"vx");
    lua_getfield(L,-2,"vy");
    ecs.velocity[id] = (VelocityCompoment){
        .vx = (float)luaL_optnumber(L,-2,0.0),
        .vy = (float)luaL_optnumber(L,-1,0.0)
    };
    lua_pop(L,2);
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

        lua_pop(L, 4); // Καθαρίζουμε r, g, b, a
    }
    return color;
}

static void mesh_parser(lua_State * L,int id)
{
    int table_idx = lua_gettop(L);
    lua_getfield(L,table_idx,"type");
    const char* type = luaL_optstring(L,-1,"rec");
    if(strcmp(type,"rec")==0)
    {
        ecs.mesh[id].type = MESH_RECTANGLE;
    }else if (strcmp(type,"circle")==0)
    {
        ecs.mesh[id].type = MESH_CICLE;
    }else
    {
        ecs.mesh[id].type = MESH_NONE;
    }

    lua_pop(L,1);
    lua_getfield(L,table_idx,"size");

    if(!lua_istable(L,-1)) 
    {
        ecs.mesh[id].size = (Vector2){10,10};
        lua_pop(L,1);
    }else{
        int size_table_index = lua_gettop(L);
        lua_getfield(L,size_table_index,"x");
        lua_getfield(L,size_table_index,"y");
        ecs.mesh[id].size = (Vector2){
            .x = (int)luaL_optinteger(L,-2,1),
            .y = (int)luaL_optinteger(L,-1,1)
        };
        lua_pop(L,3);
    }
    lua_getfield(L,table_idx,"color");
    ecs.mesh[id].color = parse_color(L,-1);
    lua_pop(L,1);
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
    uint32_t mask_bit;
    void (*parser)(lua_State*L,int id); 
}CompomentParser;


//Because of the generic struct now i can create a table of structs and skip the "spagety code"
static const CompomentParser COMPOMENT_PARSERS[] ={
    {"position",COMPOMENT_POSITION,position_parser},
    {"velocity",COMPOMENT_VELOCITY,velocity_parser},
    {"mesh",COMPOMENT_MESH,mesh_parser}
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
    int id = GetNextFreeID();
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
    //I init the mask in No Compoment
    uint32_t mask = COMPOMENT_NONE;
    //I look the table of parsers
    for(size_t i=0;i<PARSER_COUNT;i++)
    {
        //i look if there are any of the keywords in the table the user pass in create_entity 
        lua_getfield(L,1,COMPOMENT_PARSERS[i].key);
        if(!lua_isnil(L,-1)) //check if the getfield return no nil value in the first level in stack 
        {
            COMPOMENT_PARSERS[i].parser(L,id); //call the parser
            mask |= COMPOMENT_PARSERS[i].mask_bit; //create the mask bit
        }
        lua_pop(L,1); //return the stack in the original form
    }

    ecs.entinty_bitmask[id]=mask; //put the mask in the global table
    lua_pushinteger(L,id); //put in stack the id and return
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
    int id = (int)luaL_checkinteger(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    if (id >= 0 && id < MAX_ENTITIES) {
        ecs.position[id].x = x;
        ecs.position[id].y = y;
    }
    return 0;
}

int C_SetVelocity(lua_State * L)
{
    int id = (int)luaL_checkinteger(L,1);
    float vx = (float)luaL_checknumber(L,2);
    float vy = (float)luaL_checknumber(L,3);
    if(id>=0 && id<MAX_ENTITIES)
    {
        ecs.velocity[id].vx = vx;
        ecs.velocity[id].vy = vy;
    }
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
    {NULL, NULL}
};


void RegisterEngineFunctions(lua_State *L) {
    luaL_newlib(L, engine_funcs);
    lua_setglobal(L, "Engine");
}