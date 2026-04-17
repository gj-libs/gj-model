#include "formats/stl.h"
#include <stdio.h>

int main() {
    const char *filename = "assets/binary.stl";
    int count;
    float *data = stl_open(filename, &count);
    printf("count: %d\n", count);
    return 0;
}
