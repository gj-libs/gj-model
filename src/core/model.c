#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/common.h"
#include "gj_model/gj_model.h"
#include "formats/stl.h"
#include "formats/obj.h"

struct gjModel *gj_model_load(const char *filename) {
    char *ext = strrchr(filename, '.');
    struct gjModel *model = malloc_model(1);
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

struct gjModel *malloc_model(int n) {
    struct gjModel *model = malloc(n * sizeof(struct gjModel));
    for (int i = 0; i < n; i++) {
        model[i].sceneIndex = 0;
        model[i].scenes = NULL;
        model[i].sceneCount = 0;
        model[i].nodes = NULL;
        model[i].nodeCount = 0;
        model[i].meshes = NULL;
        model[i].meshCount = 0;
        model[i].materials = NULL;
        model[i].materialCount = 0;
    }
    return model;
}

struct gjScene *malloc_scene(int n) {
    struct gjScene *scene = malloc(n * sizeof(struct gjScene));
    for (int i = 0; i < n; i++) {
        memset(scene[i].name, 0, sizeof(scene[i].name));
        scene[i].nodeIndices = NULL;
        scene[i].nodeCount = 0;
    }
    return scene;
}

struct gjNode *malloc_node(int n) {
    struct gjNode *node = malloc(n * sizeof(struct gjNode));
    for (int i = 0; i < n; i++) {
        mat4_identity(node[i].transform);
        node[i].meshIndices = NULL;
        node[i].meshCount = 0;
        node[i].childIndices = NULL;
        node[i].childCount = 0;
    }
    return node;
}

struct gjMesh *malloc_mesh(int n) {
    struct gjMesh *mesh = malloc(n * sizeof(struct gjMesh));
    for (int i = 0; i < n; i++) {
        memset(mesh[i].name, 0, sizeof(mesh[i].name));
        mesh[i].positions = NULL;
        mesh[i].normals = NULL;
        mesh[i].texcoords = NULL;
        mesh[i].indices = NULL;
        mesh[i].vertexCount = 0;
        mesh[i].indexCount = 0;
        mesh[i].materialIndex = -1;
    }
    return mesh;
}

struct gjMaterial *malloc_material(int n) {
    struct gjMaterial *material = calloc(0, n * sizeof(struct gjMaterial));
    return material;
}

void free_node(struct gjNode *node) {
    if (!node) return;
    if (node->childCount > 0)
        free(node->childIndices);
    if (node->meshCount > 0)
        free(node->meshIndices);
}

void gj_model_free(struct gjModel *model) {
    for (int i = 0; i < model->sceneCount; i++) {
        if (model->scenes[i].nodeIndices)
            free(model->scenes[i].nodeIndices);
    }
    for (int i = 0; i < model->nodeCount; i++) {
        free_node(&model->nodes[i]);
    }
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
