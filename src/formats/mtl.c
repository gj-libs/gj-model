#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "formats/mtl.h"

void mtl_parse_line(const char *line, struct mtlData **materials, int *materialCount) {
    struct mtlData *mtlData = NULL;
    if (*materialCount > 0) {
        mtlData = &(*materials)[*materialCount-1];
    }
    /* BASIC MATERIALS */
    if (strncmp(line, "newmtl ", 7) == 0) {       // new material
        struct mtlData *tmp = realloc(
            *materials, sizeof(struct mtlData) * (*materialCount + 1));
        if (!tmp) {
            printf("Out of memory\n");
            return;
        }
        *materials = tmp;
        mtlData = &(*materials)[*materialCount];
        (*materialCount)++;
        memset(mtlData, 0, sizeof(struct mtlData));
        sscanf(line, "newmtl %255s", mtlData->name);
        return;
    } if (!mtlData) {
            return;
    } else if (strncmp(line, "Ka ", 3) == 0) {    // ambient color
        sscanf(line, "Ka %f %f %f",
               &mtlData->ambient[0],
               &mtlData->ambient[1],
               &mtlData->ambient[2]);
    } else if (strncmp(line, "Kd ", 3) == 0) {    // diffuse color
        sscanf(line, "Kd %f %f %f",
               &mtlData->diffuse[0],
               &mtlData->diffuse[1],
               &mtlData->diffuse[2]);
    } else if (strncmp(line, "Ks ", 3) == 0) {    // specular color
        sscanf(line, "Ks %f %f %f",
               &mtlData->specular[0],
               &mtlData->specular[1],
               &mtlData->specular[2]);
    } else if (strncmp(line, "Ns ", 3) == 0) {    // specular weight
        sscanf(line, "Ns %f", &mtlData->specularW);
    } else if (strncmp(line, "d ", 2) == 0) {     // transparency (dissolve)
        float d = 0.f;
        sscanf(line, "d %f", &d);
        mtlData->transparency = 1.f - d;
    } else if (strncmp(line, "Tr ", 3) == 0) {     // trans inverted (1-d)
        sscanf(line, "Tr %f", &mtlData->transparency);
    } else if (strncmp(line, "Tf xyz ", 7) == 0) { // transmission filter (CIEXYZ)
    } else if (strncmp(line, "Tf ", 3) == 0) {     // transmission filter (RGB)
        sscanf(line, "Tf %f %f %f",
               &mtlData->transmissionFilter[0],
               &mtlData->transmissionFilter[1],
               &mtlData->transmissionFilter[2]);
    } else if (strncmp(line, "Ni ", 3) == 0) {    // optical density
        sscanf(line, "Ni %f", &mtlData->opticalDensity);
    } else if (strncmp(line, "illum ", 6) == 0) { // illumination model
        sscanf(line, "illum %d", &mtlData->illumination);
    }
    /* TEXTURE MAPS */ /* TODO: Add options handling */
      else if (strncmp(line, "map_Ka ", 7) == 0) {   // ambient
        sscanf(line, "map_Ka %255s", mtlData->ambientMap);
    } else if (strncmp(line, "map_Kd ", 7) == 0) {   // diffuse
        sscanf(line, "map_Kd %255s", mtlData->diffuseMap);
    } else if (strncmp(line, "map_Ks ", 7) == 0) {   // specular
        sscanf(line, "map_Ks %255s", mtlData->specularMap);
    } else if (strncmp(line, "map_Ns ", 7) == 0) {   // specular highlight
        sscanf(line, "map_Ns %255s", mtlData->specularHighlightMap);
    } else if (strncmp(line, "map_d ", 6) == 0) {    // alpha
        sscanf(line, "map_d %255s", mtlData->alphaMap);
    } else if (strncmp(line, "map_bump ", 9) == 0) { // bump
        sscanf(line, "map_bump %255s", mtlData->bumpMap);
    } else if (strncmp(line, "bump ", 5) == 0) {     // bump
        sscanf(line, "bump %255s", mtlData->bumpMap);
    } else if (strncmp(line, "disp ", 5) == 0) {     // displacement
        sscanf(line, "disp %255s", mtlData->displacementMap);
    } else if (strncmp(line, "decal ", 6) == 0) {    // stencil decal
        sscanf(line, "decal %255s", mtlData->stencilMap);
    }
}

struct mtlData *mtl_open(const char *filename, int *materialCount) {
    FILE *fptr;

    if (!(fptr = fopen(filename, "rb"))) {
        printf("Failed to open file %s\n", filename);
        return NULL;
    }

    struct mtlData *materials = NULL;
    *materialCount = 0;

    char line[256];
    while (fgets(line, sizeof(line), fptr)) {
        mtl_parse_line(line, &materials, materialCount);
    }
    return materials;
}
