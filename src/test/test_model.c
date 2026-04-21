#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gj_model/gj_model.h"
#include "formats/stl.h"
#include "formats/obj.h"
#include "formats/mtl.h"

void print_node(struct gjNode *node) {
    if (!node) return;
    for (int i = 0; i < node->childCount; i++) {
        print_node(&node->children[i]);
    }
    printf("Node: %d meshes, %d children\n", node->meshCount, node->childCount);
    for (int i = 0; i < node->meshCount; i++) {
        printf("  Mesh index: %d\n", node->meshIndices[i]);
    }
}

void gj_model_print(struct gjModel *model) {
    print_node(&model->root);
    printf("Model: meshes: %d materials: %d\n", model->meshCount, model->materialCount);
    if (model->meshCount > 0) {
        for (int i = 0; i < model->meshCount; i++) {
            printf("Mesh[%d]: %d vertices, %d indices, materialIndex: %d\n",
                   i,
                   model->meshes[i].vertexCount,
                   model->meshes[i].indexCount,
                   model->meshes[i].materialIndex);
        }
    }
}

void stl_test(const char *filename) {
    printf("\nSTL test entry\n");
    printf("file: %s\n", filename);
    struct gjModel *model = gj_model_load(filename);
    // printf("Number of meshes: %d\n", model->meshCount);
    // for (int i = 0; i < model->meshCount; i++) {
    //     printf("Number of vertices for mesh[%d]: %d\n", i, model->meshes[i].vertexCount);
    // }
    // printf("Number of materials: %d\n", model->materialCount);
    gj_model_print(model);

    gj_model_free(model);
    printf("STL test exit\n");
}

void mtl_test(const char *filename) {
    printf("\nMTL test entry\n");
    printf("file: %s\n", filename);

    int materialCount = 0;
    struct gjMaterial *materials;
    materials = mtl_open(filename, &materialCount);
    for (int i = 0; i < materialCount; i++) {
        printf("newmtl %s\n", materials[i].name);
        printf("Ka %f %f %f\n",
               materials[i].ambient[0],
               materials[i].ambient[1],
               materials[i].ambient[2]);
        printf("Kd %f %f %f\n",
               materials[i].diffuse[0],
               materials[i].diffuse[1],
               materials[i].diffuse[2]);
        printf("Ks %f %f %f\n",
               materials[i].specular[0],
               materials[i].specular[1],
               materials[i].specular[2]);
        printf("Ns %f\n", materials[i].specularW);
        printf("d %f\n", materials[i].transparency);
        printf("Tr %f\n", materials[i].transparency);
        printf("Tf %f %f %f\n",
               materials[i].transmissionFilter[0],
               materials[i].transmissionFilter[1],
               materials[i].transmissionFilter[2]);
        printf("Ni %f\n", materials[i].opticalDensity);
        printf("illum %d\n", materials[i].illumination);
        printf("map_Ka %s\n", materials[i].ambientMap);
        printf("map_Kd %s\n", materials[i].diffuseMap);
        printf("map_Ka %s\n", materials[i].specularMap);
        printf("map_Ns %s\n", materials[i].specularHighlightMap);
        printf("map_d %s\n", materials[i].alphaMap);
        printf("map_bump %s\n", materials[i].bumpMap);
        printf("bump %s\n", materials[i].bumpMap);
        printf("disp %s\n", materials[i].displacementMap);
        printf("decal %s\n", materials[i].stencilMap);
        printf("\n");
    }
    free(materials);

    printf("MTL test exit\n");
}

void obj_test(const char *filename) {
    printf("\nOBJ test entry\n");
    printf("file: %s\n", filename);

    struct gjModel *model = gj_model_load(filename);
    printf("Number of meshes: %d\n", model->meshCount);
    for (int i = 0; i < model->meshCount; i++) {
        printf("Number of vertices for mesh[%d]: %d\n", i, model->meshes[i].vertexCount);
    }
    printf("Number of materials: %d\n", model->materialCount);
    gj_model_free(model);

    printf("OBJ test exit\n");
}

int main() {
    stl_test("assets/binary.stl");
    stl_test("assets/ascii.stl");
    obj_test("assets/cube/cube.obj");
    mtl_test("assets/objTest/2nrtbod1out.mtl");
    return 0;
}
