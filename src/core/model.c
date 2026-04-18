#include <stdlib.h>
#include <string.h>
#include "gj_model/gj_model.h"
#include "formats/stl.h"
#include "formats/obj.h"

struct Mesh gj_model_load(const char *filename) {
    char *ext = strrchr(filename, '.');
    struct Mesh nullMesh = {0};
    if (ext == NULL) {
        // gj_set_error("No file extension found in \"%s\"\n", filename);
        return nullMesh;
    }

    if (strcasecmp(ext, ".stl") == 0) {
        return stl_open(filename);
    } else if (strcasecmp(ext, ".obj") == 0) {
        return obj_open(filename);
    } else {
        // gj_set_error("Unsupported file format \"%s\"\n", ext);
        return nullMesh;
    }
}

void gj_model_free(struct Mesh *mesh) {
    free(mesh->vertices);
}
