#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gj_model/gj_model.h"
#include "formats/stl.h"
#include "formats/obj.h"

struct gjModel *gj_model_load(const char *filename) {
    char *ext = strrchr(filename, '.');
    struct gjModel *model = malloc_model();
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

struct gjModel *malloc_model() {
    struct gjModel *model = malloc(sizeof(struct gjModel));
    model->root = (struct gjNode){0};
    model->meshes = NULL;
    model->meshCount = 0;
    model->materials = NULL;
    model->materialCount = 0;
    return model;
}

struct gjNode *malloc_node() {
    struct gjNode *node = malloc(sizeof(struct gjNode));
    node->meshIndices = NULL;
    node->meshCount = 0;
    node->children = NULL;
    node->childCount = 0;
    return node;
}

struct gjMesh *malloc_mesh() {
    struct gjMesh *mesh = malloc(sizeof(struct gjMesh));
    mesh->positions = NULL;
    mesh->normals = NULL;
    mesh->texcoords = NULL;
    mesh->indices = NULL;
    mesh->vertexCount = 0;
    mesh->indexCount = 0;
    mesh->materialIndex = -1;
    return mesh;
}

struct gjMaterial *malloc_material() {
    struct gjMaterial *material = calloc(0, sizeof(struct gjMaterial));
    return material;
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
            if (model->meshes[i].positions)
                free(model->meshes[i].positions);
            if (model->meshes[i].normals)
                free(model->meshes[i].normals);
            if (model->meshes[i].texcoords)
                free(model->meshes[i].texcoords);
            if (model->meshes[i].indices)
                free(model->meshes[i].indices);
        }
        free(model->meshes);
    }
    if (model->materialCount > 0)
        free(model->materials);
    free(model);
}
