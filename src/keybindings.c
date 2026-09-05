#include <string.h>
#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include "cJSON.h"

typedef struct
{
    const char *key_name;
    KeyboardKey raylib_key;
} KeyBindings;


static const KeyBindings KEY_LOOKUP_TABLE[] ={
    {"KEY_A", KEY_A},
    {"KEY_APOSTROPHE", KEY_APOSTROPHE},
    {"KEY_B", KEY_B},
    {"KEY_BACK", KEY_BACK},
    {"KEY_BACKSLASH", KEY_BACKSLASH},
    {"KEY_BACKSPACE", KEY_BACKSPACE},
    {"KEY_C", KEY_C},
    {"KEY_CAPS_LOCK", KEY_CAPS_LOCK},
    {"KEY_COMMA", KEY_COMMA},
    {"KEY_D", KEY_D},
    {"KEY_DELETE", KEY_DELETE},
    {"KEY_DOWN", KEY_DOWN},
    {"KEY_E", KEY_E},
    {"KEY_END", KEY_END},
    {"KEY_ENTER", KEY_ENTER},
    {"KEY_EQUAL", KEY_EQUAL},
    {"KEY_ESCAPE", KEY_ESCAPE},
    {"KEY_F", KEY_F},
    {"KEY_F1", KEY_F1},
    {"KEY_F10", KEY_F10},
    {"KEY_F11", KEY_F11},
    {"KEY_F12", KEY_F12},
    {"KEY_F2", KEY_F2},
    {"KEY_F3", KEY_F3},
    {"KEY_F4", KEY_F4},
    {"KEY_F5", KEY_F5},
    {"KEY_F6", KEY_F6},
    {"KEY_F7", KEY_F7},
    {"KEY_F8", KEY_F8},
    {"KEY_F9", KEY_F9},
    {"KEY_FIVE", KEY_FIVE},
    {"KEY_FOUR", KEY_FOUR},
    {"KEY_G", KEY_G},
    {"KEY_GRAVE", KEY_GRAVE},
    {"KEY_H", KEY_H},
    {"KEY_HOME", KEY_HOME},
    {"KEY_I", KEY_I},
    {"KEY_INSERT", KEY_INSERT},
    {"KEY_J", KEY_J},
    {"KEY_K", KEY_K},
    {"KEY_KB_MENU", KEY_KB_MENU},
    {"KEY_KP_0", KEY_KP_0},
    {"KEY_KP_1", KEY_KP_1},
    {"KEY_KP_2", KEY_KP_2},
    {"KEY_KP_3", KEY_KP_3},
    {"KEY_KP_4", KEY_KP_4},
    {"KEY_KP_5", KEY_KP_5},
    {"KEY_KP_6", KEY_KP_6},
    {"KEY_KP_7", KEY_KP_7},
    {"KEY_KP_8", KEY_KP_8},
    {"KEY_KP_9", KEY_KP_9},
    {"KEY_KP_ADD", KEY_KP_ADD},
    {"KEY_KP_DECIMAL", KEY_KP_DECIMAL},
    {"KEY_KP_DIVIDE", KEY_KP_DIVIDE},
    {"KEY_KP_ENTER", KEY_KP_ENTER},
    {"KEY_KP_EQUAL", KEY_KP_EQUAL},
    {"KEY_KP_MULTIPLY", KEY_KP_MULTIPLY},
    {"KEY_KP_SUBTRACT", KEY_KP_SUBTRACT},
    {"KEY_L", KEY_L},
    {"KEY_LEFT", KEY_LEFT},
    {"KEY_LEFT_ALT", KEY_LEFT_ALT},
    {"KEY_LEFT_BRACKET", KEY_LEFT_BRACKET},
    {"KEY_LEFT_CONTROL", KEY_LEFT_CONTROL},
    {"KEY_LEFT_SHIFT", KEY_LEFT_SHIFT},
    {"KEY_LEFT_SUPER", KEY_LEFT_SUPER},
    {"KEY_M", KEY_M},
    {"KEY_MENU", KEY_MENU},
    {"KEY_MINUS", KEY_MINUS},
    {"KEY_N", KEY_N},
    {"KEY_NINE", KEY_NINE},
    {"KEY_NULL", KEY_NULL},
    {"KEY_NUM_LOCK", KEY_NUM_LOCK},
    {"KEY_O", KEY_O},
    {"KEY_ONE", KEY_ONE},
    {"KEY_P", KEY_P},
    {"KEY_PAGE_DOWN", KEY_PAGE_DOWN},
    {"KEY_PAGE_UP", KEY_PAGE_UP},
    {"KEY_PAUSE", KEY_PAUSE},
    {"KEY_PERIOD", KEY_PERIOD},
    {"KEY_PRINT_SCREEN", KEY_PRINT_SCREEN},
    {"KEY_Q", KEY_Q},
    {"KEY_R", KEY_R},
    {"KEY_RIGHT", KEY_RIGHT},
    {"KEY_RIGHT_ALT", KEY_RIGHT_ALT},
    {"KEY_RIGHT_BRACKET", KEY_RIGHT_BRACKET},
    {"KEY_RIGHT_CONTROL", KEY_RIGHT_CONTROL},
    {"KEY_RIGHT_SHIFT", KEY_RIGHT_SHIFT},
    {"KEY_RIGHT_SUPER", KEY_RIGHT_SUPER},
    {"KEY_S", KEY_S},
    {"KEY_SCROLL_LOCK", KEY_SCROLL_LOCK},
    {"KEY_SEMICOLON", KEY_SEMICOLON},
    {"KEY_SEVEN", KEY_SEVEN},
    {"KEY_SIX", KEY_SIX},
    {"KEY_SLASH", KEY_SLASH},
    {"KEY_SPACE", KEY_SPACE},
    {"KEY_T", KEY_T},
    {"KEY_TAB", KEY_TAB},
    {"KEY_THREE", KEY_THREE},
    {"KEY_TWO", KEY_TWO},
    {"KEY_U", KEY_U},
    {"KEY_UP", KEY_UP},
    {"KEY_V", KEY_V},
    {"KEY_VOLUME_DOWN", KEY_VOLUME_DOWN},
    {"KEY_VOLUME_UP", KEY_VOLUME_UP},
    {"KEY_W", KEY_W},
    {"KEY_X", KEY_X},
    {"KEY_Y", KEY_Y},
    {"KEY_Z", KEY_Z},
    {"KEY_ZERO", KEY_ZERO}
};
#define KEY_LOOKUP_SIZE (sizeof(KEY_LOOKUP_TABLE)/sizeof(KEY_LOOKUP_TABLE[0]))

static inline int comparer(const void *key,const void *elem)
{
    return strcmp((const char * )key,((const KeyBindings*)elem)->key_name);
}


KeyboardKey GetRaylibKey(const char * key)
{
    if(key == NULL) return KEY_NULL;
    KeyBindings* found = bsearch(key,KEY_LOOKUP_TABLE,KEY_LOOKUP_SIZE,sizeof(KeyBindings),comparer);
    if(found!=NULL) return found->raylib_key;
    return KEY_NULL;
}

