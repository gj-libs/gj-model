#include <stdlib.h>
#include <string.h>
#include "gj_model/gj_model.h"
#include "formats/stl.h"

float *gj_model_load(const char *filename, int *count) {
    char *ext = strrchr(filename, '.');
    if (ext == NULL) {
        // gj_set_error("No file extension found in \"%s\"\n", filename);
        return NULL;
    }

    if (strcasecmp(ext, ".stl") == 0) {
        return stl_open(filename, count);
    } else {
        // gj_set_error("Unsupported file format \"%s\"\n", ext);
        return NULL;
    }
    return NULL;
}

void gj_model_free(float *data) {
    free(data);
}
