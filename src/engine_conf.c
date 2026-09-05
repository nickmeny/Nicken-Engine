#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"
#include "Map.h"
#include "keybindings.h"
#include"engine_conf.h"



EngineConfig GetDefaultConfig(void)
{
    return (EngineConfig){
        .KeyBindingsNameFile = "keys.json",
        .targetFPS = 60,
        .windowHeight = 500,
        .windowWidth = 500
    };
}

static void BindKey(Map map,char * action_name,char * key_name)
{
    KeyboardKey* key = malloc(sizeof(*key));
    *key = GetRaylibKey(key_name);
    char *action_copy = strdup(action_name);
    map_insert(map,action_copy,key);
}

static const char * GetConfigFile(const char *dirPath)
{
    FilePathList files = LoadDirectoryFilesEx(dirPath,".nik",false);
    const char * foundFile = NULL;
    if(files.count>0)
    {
        foundFile = files.paths[0];
    }
    const char * copy_path = NULL;
    if(foundFile!=NULL)
    {
        copy_path = strdup(foundFile);
    }
    UnloadDirectoryFiles(files);
    return copy_path;
}


void LoadEngineConfigs(const char * filename,EngineConfig * config)
{
    bool is_null = false;
    if(filename==NULL)
    {
        filename = GetConfigFile(GetApplicationDirectory());
        is_null = true;
    }
    printf("Try to load settings from %s\n",filename);
    FILE * file = fopen(filename,"r");
    if(!file)
    {
        printf("No Settings found for the Engine\n");
        return;
    }
    char line[128];
    char key[64];
    char value[64];
    while(fgets(line,sizeof(line),file))
    {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if(sscanf(line,"%63s %63s",key,value)==2)
        {
            if(strcmp(key,"target_fps")==0) config->targetFPS=atoi(value);
            else if(strcmp(key,"window_width")==0) config->windowWidth = atoi(value);
            else if(strcmp(key,"window_height")==0) config->windowHeight = atoi(value);
            else if(strcmp(key,"key_bindings_file")==0) config->KeyBindingsNameFile = strdup(value);
        }
    }
    fclose(file);
    printf("Engine Configs load succesfully\n");
    printf("The Settings are:\n\tWindow Height: %d,\n\tWindow Width: %d,\n\t Target FPS: %d,\n\t Key Bindings File: %s\n",
        config->windowHeight,
        config->windowWidth,
        config->targetFPS,
        config->KeyBindingsNameFile
    );
    if(is_null)
    {
        free((void*)filename);
    }
}
void SaveEngineConfig(const char * filename,const EngineConfig* config)
{
    printf("Save Engine Config to %s\n",filename);
    EngineConfig default_config;
    if(config==NULL)
    {
        default_config = GetDefaultConfig();
        config=&default_config;
    }
    FILE *file = fopen(filename,"w");
    if(!file) return;
    fprintf(file,"#Engine Settings Generated automatically from engine\n");
    fprintf(file,"#Display\n");
    fprintf(file,"window_width  %d\n",config->windowWidth);
    fprintf(file,"window_height %d\n",config->windowHeight);
    fprintf(file,"target_fps %d\n",config->targetFPS);
    fprintf(file,"#Files\n");
    fprintf(file,"key_bindings_file %s\n",config->KeyBindingsNameFile);
    fclose(file);
    printf("Save engine config was successfully\n");

}

void SaveKeyBindings(const char* filename)
{
    FILE * fp = fopen(filename,"w");
    if(!fp) return;
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json,"Jump","KEY_SPACE");
    cJSON_AddStringToObject(json,"Right","KEY_D");
    cJSON_AddStringToObject(json,"Left","KEY_A");
    char *json_str = cJSON_Print(json);
    fputs(json_str,fp);
    fclose(fp);
    cJSON_free(json_str);
    cJSON_Delete(json);
}


void LoadKeyBindings(const char* filename,Map map)
{
    FILE * fp = fopen(filename,"r");
    if(!fp) return;
    char buffer[1025];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, fp);
    fclose(fp);
    buffer[bytes_read]='\0';
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "[ERROR] JSON Parse Error before: %s\n", error_ptr);
        }
        return;
    }
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, json) {
        if (item->string != NULL) {
            // Check value type and handle accordingly
            if (cJSON_IsString(item) && (item->valuestring != NULL)) {
                printf("Action: %-10s -> Key: %s\n", item->string, item->valuestring);
                BindKey(map,item->string, item->valuestring);
            } 
        }
    }
    cJSON_Delete(json);
    
}