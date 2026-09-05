#pragma once 
#include "Map.h"

struct engine_config
{
    char * KeyBindingsNameFile;
    int targetFPS;
    int windowHeight;
    int windowWidth;
};
typedef struct engine_config EngineConfig;
void LoadEngineConfigs(const char * filename,EngineConfig * config);
void LoadKeyBindings(const char* filename,Map map);
void SaveEngineConfig(const char * filename,const EngineConfig* config);
void SaveKeyBindings(const char* filename);
EngineConfig GetDefaultConfig(void);