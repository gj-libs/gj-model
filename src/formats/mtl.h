#ifndef MTL_H
#define MTL_H

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

struct mtlData *mtl_open(const char *filename, int *materialCount);

#endif  // MTL_H
