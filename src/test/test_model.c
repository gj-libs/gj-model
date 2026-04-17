#include "formats/stl.h"
#include "formats/obj.h"
#include <stdio.h>

void stl_test() {
    const char *filename = "assets/ascii.stl";
    int count;
    stl_open(filename, &count);
    printf("count: %d\n", count);
}

void obj_test() {
    const char *filename = "assets/cube/cube.obj";
    int count;
    obj_open(filename, &count);
    printf("count: %d\n", count);
}

int main() {
    obj_test();
    return 0;
}
