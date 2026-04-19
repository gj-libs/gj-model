#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "formats/mtl.h"
#include "gj_model/gj_model.h"

void mtl_parse_line(const char *line, struct gjMaterial **materials, int *materialCount) {
    struct gjMaterial *mat = NULL;
    if (*materialCount > 0) {
        mat = &(*materials)[*materialCount-1];
    }
    /* BASIC MATERIALS */
    if (strncmp(line, "newmtl ", 7) == 0) {       // new material
        struct gjMaterial *tmp = realloc(
            *materials, sizeof(struct gjMaterial) * (*materialCount + 1));
        if (!tmp) {
            printf("Out of memory\n");
            return;
        }
        *materials = tmp;
        mat = &(*materials)[*materialCount];
        (*materialCount)++;
        memset(mat, 0, sizeof(struct gjMaterial));
        sscanf(line, "newmtl %255s", mat->name);
        return;
    } if (!mat) {
            return;
    } else if (strncmp(line, "Ka ", 3) == 0) {    // ambient color
        sscanf(line, "Ka %f %f %f",
               &mat->ambient[0],
               &mat->ambient[1],
               &mat->ambient[2]);
    } else if (strncmp(line, "Kd ", 3) == 0) {    // diffuse color
        sscanf(line, "Kd %f %f %f",
               &mat->diffuse[0],
               &mat->diffuse[1],
               &mat->diffuse[2]);
    } else if (strncmp(line, "Ks ", 3) == 0) {    // specular color
        sscanf(line, "Ks %f %f %f",
               &mat->specular[0],
               &mat->specular[1],
               &mat->specular[2]);
    } else if (strncmp(line, "Ns ", 3) == 0) {    // specular weight
        sscanf(line, "Ns %f", &mat->specularW);
    } else if (strncmp(line, "d ", 2) == 0) {     // transparency (dissolve)
        float d = 0.f;
        sscanf(line, "d %f", &d);
        mat->transparency = 1.f - d;
    } else if (strncmp(line, "Tr ", 3) == 0) {     // trans inverted (1-d)
        sscanf(line, "Tr %f", &mat->transparency);
    } else if (strncmp(line, "Tf xyz ", 7) == 0) { // transmission filter (CIEXYZ)
    } else if (strncmp(line, "Tf ", 3) == 0) {     // transmission filter (RGB)
        sscanf(line, "Tf %f %f %f",
               &mat->transmissionFilter[0],
               &mat->transmissionFilter[1],
               &mat->transmissionFilter[2]);
    } else if (strncmp(line, "Ni ", 3) == 0) {    // optical density
        sscanf(line, "Ni %f", &mat->opticalDensity);
    } else if (strncmp(line, "illum ", 6) == 0) { // illumination model
        sscanf(line, "illum %d", &mat->illumination);
    }
    /* TEXTURE MAPS */ /* TODO: Add options handling */
      else if (strncmp(line, "map_Ka ", 7) == 0) {   // ambient
        sscanf(line, "map_Ka %255s", mat->ambientMap);
    } else if (strncmp(line, "map_Kd ", 7) == 0) {   // diffuse
        sscanf(line, "map_Kd %255s", mat->diffuseMap);
    } else if (strncmp(line, "map_Ks ", 7) == 0) {   // specular
        sscanf(line, "map_Ks %255s", mat->specularMap);
    } else if (strncmp(line, "map_Ns ", 7) == 0) {   // specular highlight
        sscanf(line, "map_Ns %255s", mat->specularHighlightMap);
    } else if (strncmp(line, "map_d ", 6) == 0) {    // alpha
        sscanf(line, "map_d %255s", mat->alphaMap);
    } else if (strncmp(line, "map_bump ", 9) == 0) { // bump
        sscanf(line, "map_bump %255s", mat->bumpMap);
    } else if (strncmp(line, "bump ", 5) == 0) {     // bump
        sscanf(line, "bump %255s", mat->bumpMap);
    } else if (strncmp(line, "disp ", 5) == 0) {     // displacement
        sscanf(line, "disp %255s", mat->displacementMap);
    } else if (strncmp(line, "decal ", 6) == 0) {    // stencil decal
        sscanf(line, "decal %255s", mat->stencilMap);
    }
}

struct gjMaterial *mtl_open(const char *filename, int *materialCount) {
    FILE *fptr;

    if (!(fptr = fopen(filename, "rb"))) {
        printf("Failed to open file %s\n", filename);
        return NULL;
    }

    struct gjMaterial *materials = NULL;
    *materialCount = 0;

    char line[256];
    while (fgets(line, sizeof(line), fptr)) {
        mtl_parse_line(line, &materials, materialCount);
    }
    return materials;
}
