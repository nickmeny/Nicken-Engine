#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "GUI/node.h"
#include "Vector.h"

bool Export(Vector entities, const char *filename)
{
    if (!entities || !filename) return false;

    FILE *file = fopen(filename, "w");
    if (!file) return false;

    fprintf(file, "entity = Engine.create_entity({\n");
    int i=0;
    for(VectorNode vn=vector_first(entities);vn!=VECTOR_EOF;vn=vector_next(entities,vn))
    {
        Node *node = (Node*)vector_get_at(entities, i);
        if (!node) continue;


        const char *type = "none";
        float size[2] = { 0.0f, 0.0f };

        switch (node->type)
        {
            case NODE_TYPE_COLLISION:
                if (node->collision.type == COLLISION_BOX)
                {
                    type = "rec";
                    size[0] = node->collision.box_bounds.width; 
                    size[1] = node->collision.box_bounds.height;
                }
                else if (node->collision.type == COLLISION_CIRCLE)
                {
                    type = "circle";
                    size[0] = node->collision.circle_radius;
                    size[1] = 0.0f;
                }

                fprintf(file, "    collision = {\n");
                fprintf(file, "        type = \"%s\",\n", type);
                fprintf(file, "        size = { x = %.2f, y = %.2f },\n", size[0], size[1]);
                fprintf(file, "        layer = 1,\n");
                fprintf(file, "        mask = 1\n");
                fprintf(file, "    }");
                break;

            case NODE_TYPE_MESH:
                
                if (node->mesh.type == MESH_TYPE_BOX)
                {
                    type = "rec";
                    size[0] = node->mesh.box_bounds.width;
                    size[1] = node->mesh.box_bounds.height;
                }
                else if (node->mesh.type == MESH_TYPE_CIRCLE)
                {
                    type = "circle";
                    size[0] = node->mesh.circle_radius; 
                    size[1] = 0.0f;
                }

                fprintf(file, "    mesh = {\n");
                fprintf(file, "        type = \"%s\",\n", type);
                fprintf(file, "        size = { x = %.2f, y = %.2f },\n", size[0], size[1]);
                fprintf(file, "        color = \"YELLOW\"\n");
                fprintf(file, "    }");
                break;

            default:
                break;
        }
        if(vector_next(entities,vn)!=VECTOR_EOF) fprintf(file,",");
        fprintf(file,"\n");
        i++;
    }
    fprintf(file, "})\n\n");

    fclose(file);
    return true;
}