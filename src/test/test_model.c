#include "formats/stl.h"
#include <stdio.h>

int main() {
    const char *filename = "assets/ascii.stl";
    int count;
    stl_open(filename, &count);
    printf("count: %d\n", count);
    return 0;
}
