#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine_conf.h"
#include <stdbool.h>
#include "GUI/editor.h"

typedef int (*function_ptr)(int agrc,char *argv[]);

typedef struct command {
    char * name;
    function_ptr function;
}Command;

typedef const Command *command_ptr;

int run_game(int argc,char * argv[])
{
    if (argc < 1) {
        printf("Χρήση: %s <script.lua>\n", argv[0]);
        return 1;
    }
    return 2;
}

int init_engine(int argc,char * argv[])
{
    char default_name[9] ="init.nik";
    bool use_defualt_value = false;
    if(argc<2) 
    {
        fprintf(stderr,"[Warning] You have to specify the init filename\n");
        use_defualt_value = true;
    }
    if(use_defualt_value){
        SaveEngineConfig(default_name,NULL);
        SaveKeyBindings("keys.json"); 
        return 0;
    }
    SaveEngineConfig(argv[1],NULL);
    if(argc==3)
    {
        SaveKeyBindings(argv[2]);
        return 0;
    }
    return 0;
}

int run_editor(int argc,char * argv[])
{
    //cast to void becaue they are unused and i get warnigns. ( I could have use the __attribute__((unsude)) but i want to be portable)
    (void)argc; 
    (void)argv;
    init_editor();
    return 0;
}

Command commands[] =
{
    #include "commands.h"
};

static inline int comparer(const void *key,const void *elem)
{
    return strcmp((const char * )key,((command_ptr)elem)->name);
}

int CLI(int argc, char * argv[])
{
    size_t element_size = sizeof(Command);
    Command * found = bsearch(argv[1],commands,sizeof(commands)/element_size,element_size,comparer);
    if(!found)
    {
        fprintf(stderr,"Invalid command");
        return(0);
    }
    return  found->function(argc-1,argv+1);
}