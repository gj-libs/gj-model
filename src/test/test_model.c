#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gj_model/gj_model.h"
#include "formats/stl.h"
#include "formats/obj.h"
#include "formats/mtl.h"

void stl_test(const char *filename) {
    printf("\nSTL test entry\n");
    printf("file: %s\n", filename);

    struct gjModel *model = malloc(sizeof(struct gjModel));
    memset(model, 0, sizeof(struct gjModel));
    stl_open(filename, model);
    printf("Number of meshes: %d\n", model->meshCount);
    for (int i = 0; i < model->meshCount; i++) {
        printf("Number of vertices for mesh[%d]: %d\n", i, model->meshes[i].vertexCount);
    }
    printf("Number of materials: %d\n", model->materialCount);
    gj_model_free(model);
    printf("STL test exit\n");
}

void obj_test() {
    printf("OBJ test entry\n");
    const char *objFName = "assets/objTest/2nrtbod1out.obj";
    struct gjModel model;
    obj_open(objFName, &model);
    // printf("count: %d\n", mesh.nVertices);

    const char *mtlFName = "assets/objTest/2nrtbod1out.mtl";
    int materialCount = 0;
    struct gjMaterial *materials;
    materials = mtl_open(mtlFName, &materialCount);
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
    model.materials = materials;
    model.materialCount = materialCount;
    printf("OBJ test exit\n");
}

int main() {
    stl_test("assets/binary.stl");
    stl_test("assets/ascii.stl");
    // obj_test();
    return 0;
}
