#include <stdio.h>
#include "gj_model/gj_model.h"
#include "formats/stl.h"
#include "formats/obj.h"

void stl_test() {
    const char *filename = "assets/ascii.stl";
    struct Mesh mesh = stl_open(filename);
    printf("count: %d\n", mesh.vertexCount);
}

void obj_test() {
    const char *filename = "../../game-1/assets/nar.obj";
    struct Mesh mesh = obj_open(filename);
    printf("count: %d\n", mesh.vertexCount);
}

int main() {
    obj_test();
    return 0;
}
