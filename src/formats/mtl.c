#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mtlData {
    char name[256];
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float specularW;
    float transparency;
    float transmissionFilter[3];
    float opticalDensity;
    int illumination;

    char ambientMap[256];
    char diffuseMap[256];
    char specularMap[256];
    char specularHighlightMap[256];
    char alphaMap[256];
    char bumpMap[256];
    char displacementMap[256];
    char stencilMap[256];
};

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
        printf("newmtl %s\n", mtlData->name);
        return;
    } if (!mtlData) {
            return;
    } else if (strncmp(line, "Ka ", 3) == 0) {    // ambient color
        sscanf(line, "Ka %f %f %f",
               &mtlData->ambient[0],
               &mtlData->ambient[1],
               &mtlData->ambient[2]);
        printf("Ka %f %f %f\n",
               mtlData->ambient[0],
               mtlData->ambient[1],
               mtlData->ambient[2]);
    } else if (strncmp(line, "Kd ", 3) == 0) {    // diffuse color
        sscanf(line, "Kd %f %f %f",
               &mtlData->diffuse[0],
               &mtlData->diffuse[1],
               &mtlData->diffuse[2]);
        printf("Kd %f %f %f\n",
               mtlData->diffuse[0],
               mtlData->diffuse[1],
               mtlData->diffuse[2]);
    } else if (strncmp(line, "Ks ", 3) == 0) {    // specular color
        sscanf(line, "Ks %f %f %f",
               &mtlData->specular[0],
               &mtlData->specular[1],
               &mtlData->specular[2]);
        printf("Ks %f %f %f\n",
               mtlData->specular[0],
               mtlData->specular[1],
               mtlData->specular[2]);
    } else if (strncmp(line, "Ns ", 3) == 0) {    // specular weight
        sscanf(line, "Ns %f", &mtlData->specularW);
        printf("Ns %f\n", mtlData->specularW);
    } else if (strncmp(line, "d ", 2) == 0) {     // transparency (dissolve)
        float d = 0.f;
        sscanf(line, "d %f", &d);
        mtlData->transparency = 1.f - d;
        printf("d %f\n", mtlData->transparency);
    } else if (strncmp(line, "Tr ", 3) == 0) {     // trans inverted (1-d)
        sscanf(line, "Tr %f", &mtlData->transparency);
        printf("Tr %f\n", mtlData->transparency);
    } else if (strncmp(line, "Tf xyz ", 7) == 0) { // transmission filter (CIEXYZ)
    } else if (strncmp(line, "Tf ", 3) == 0) {     // transmission filter (RGB)
        sscanf(line, "Tf %f %f %f",
               &mtlData->transmissionFilter[0],
               &mtlData->transmissionFilter[1],
               &mtlData->transmissionFilter[2]);
        printf("Tf %f %f %f\n",
               mtlData->transmissionFilter[0],
               mtlData->transmissionFilter[1],
               mtlData->transmissionFilter[2]);
    } else if (strncmp(line, "Ni ", 3) == 0) {    // optical density
        sscanf(line, "Ni %f", &mtlData->opticalDensity);
        printf("Ni %f\n", mtlData->opticalDensity);
    } else if (strncmp(line, "illum ", 6) == 0) { // illumination model
        sscanf(line, "illum %d", &mtlData->illumination);
        printf("illum %d\n", mtlData->illumination);
    }
    /* TEXTURE MAPS */ /* TODO: Add options handling */
      else if (strncmp(line, "map_Ka ", 7) == 0) {   // ambient
        sscanf(line, "map_Ka %255s", mtlData->ambientMap);
        printf("map_Ka %s\n", mtlData->ambientMap);
    } else if (strncmp(line, "map_Kd ", 7) == 0) {   // diffuse
        sscanf(line, "map_Kd %255s", mtlData->diffuseMap);
        printf("map_Kd %s\n", mtlData->diffuseMap);
    } else if (strncmp(line, "map_Ks ", 7) == 0) {   // specular
        sscanf(line, "map_Ks %255s", mtlData->specularMap);
        printf("map_Ka %s\n", mtlData->specularMap);
    } else if (strncmp(line, "map_Ns ", 7) == 0) {   // specular highlight
        sscanf(line, "map_Ns %255s", mtlData->specularHighlightMap);
        printf("map_Ns %s\n", mtlData->specularHighlightMap);
    } else if (strncmp(line, "map_d ", 6) == 0) {    // alpha
        sscanf(line, "map_d %255s", mtlData->alphaMap);
        printf("map_d %s\n", mtlData->alphaMap);
    } else if (strncmp(line, "map_bump ", 9) == 0) { // bump
        sscanf(line, "map_bump %255s", mtlData->bumpMap);
        printf("map_bump %s\n", mtlData->bumpMap);
    } else if (strncmp(line, "bump ", 5) == 0) {     // bump
        sscanf(line, "bump %255s", mtlData->bumpMap);
        printf("bump %s\n", mtlData->bumpMap);
    } else if (strncmp(line, "disp ", 5) == 0) {     // displacement
        sscanf(line, "disp %255s", mtlData->displacementMap);
        printf("disp %s\n", mtlData->displacementMap);
    } else if (strncmp(line, "decal ", 6) == 0) {    // stencil decal
        sscanf(line, "decal %255s", mtlData->stencilMap);
        printf("decal %s\n", mtlData->stencilMap);
    }
}

void mtl_open(const char *filename) {
    FILE *fptr;

    if (!(fptr = fopen(filename, "rb"))) {
        printf("Failed to open file %s\n", filename);
        return;
    }

    struct mtlData *materials = NULL;
    int materialCount = 0;

    char line[256];
    while (fgets(line, sizeof(line), fptr)) {
        mtl_parse_line(line, &materials, &materialCount);
    }
}
