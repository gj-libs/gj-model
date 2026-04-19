#include <stdio.h>
#include "gj_model/gj_model.h"
#include "formats/stl.h"
#include "formats/obj.h"
#include "formats/mtl.h"

void stl_test() {
    const char *filename = "assets/ascii.stl";
    struct Mesh mesh = stl_open(filename);
    printf("count: %d\n", mesh.vertexCount);
}

void obj_test() {
    const char *objFName = "assets/objTest/2nrtbod1out.obj";
    struct Mesh mesh = obj_open(objFName);
    printf("count: %d\n", mesh.vertexCount);

    printf("opening mtl\n");
    const char *mtlFName = "assets/objTest/2nrtbod1out.mtl";
    mtl_open(mtlFName);
}

int main() {
    obj_test();
    return 0;
}
