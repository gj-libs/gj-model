#include <stdio.h>
#include "gj_model/gj_model.h"
#include "formats/stl.h"
#include "formats/obj.h"
#include "formats/mtl.h"

void stl_test() {
    const char *filename = "assets/ascii.stl";
    struct Mesh mesh = stl_open(filename);
    printf("count: %d\n", mesh.nVertices);
}

void obj_test() {
    const char *objFName = "assets/objTest/2nrtbod1out.obj";
    struct Mesh mesh = obj_open(objFName);
    printf("count: %d\n", mesh.nVertices);

    printf("opening mtl\n");
    const char *mtlFName = "assets/objTest/2nrtbod1out.mtl";
    int materialCount = 0;
    struct mtlData *materials;
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
}

int main() {
    obj_test();
    return 0;
}
