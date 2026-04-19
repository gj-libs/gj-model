#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gj_model/gj_model.h"
#include "formats/stl.h"
#include "formats/obj.h"

struct gjModel *gj_model_load(const char *filename) {
    char *ext = strrchr(filename, '.');
    struct gjModel *model = malloc(sizeof(struct gjModel));
    if (!model) {
        printf("Out of memory!\n");
        return NULL;
    }

    if (ext == NULL) {
        return NULL;
    }

    if (strcasecmp(ext, ".stl") == 0) {
        if (stl_open(filename, model)) {
            return NULL;
        }
    } else if (strcasecmp(ext, ".obj") == 0) {
        if (obj_open(filename, model)) {
            return NULL;
        }
    } else {
        return NULL;
    }
    return model;
}

void free_node(struct gjNode *node) {
    if (!node) return;
    for (int i = 0; i < node->childCount; i++) {
        free_node(&node->children[i]);
    }
    if (node->childCount > 0)
        free(node->children);
    if (node->meshCount > 0)
        free(node->meshIndices);
}

void gj_model_free(struct gjModel *model) {
    free_node(&model->root);
    if (model->meshCount > 0) {
        for (int i = 0; i < model->meshCount; i++) {
            free(model->meshes[i].positions);
            free(model->meshes[i].normals);
            free(model->meshes[i].texcoords);
            free(model->meshes[i].indices);
        }
        free(model->meshes);
    }
    if (model->materialCount > 0)
        free(model->materials);
    free(model);
}
