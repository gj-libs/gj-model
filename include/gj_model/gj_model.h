/*
 * gj_model
 * Copyright (c) 2026 giji676
 * Licensed under MIT (see LICENSE file)
 */

#ifndef GJ_MODEL_H
#define GJ_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

struct gjMesh {
    float *positions;
    float *normals;
    float *texcoords;

    unsigned int *indices;

    int vertexCount;
    int indexCount;

    int materialIndex;
};

struct gjMaterial {
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

struct gjNode {
    float transform[16];

    int *meshIndices;
    int meshCount;

    struct gjNode *children;
    int childCount;
};

struct gjModel {
    struct gjNode root;

    struct gjMesh *meshes;
    int meshCount;

    struct gjMaterial *materials;
    int materialCount;
};

/*
 * Opens a 3D model file
 *
 * @param filename Path to the model file
 *
 * @return A struct gjModel
 *
 * TODO: Update for new system
 *
 * Mesh.vertices has 24 floats per vertice
 * in the format (x,y,z,nx,ny,nz,u,v)
 *   x,y,z    are position coords
 *   nx,ny,nz are normals
 *   u,v      are texture coords
 *
 * The returned Mesh.vertices buffer must be freed using gj_model_free().
 */
struct gjModel *gj_model_load(const char *filename);

/*
 * Frees the allocated image data
 *
 * @param data Pointer to the image data to be freed
 */
void        gj_model_free(struct gjModel *model);

#ifdef __cplusplus
}
#endif

#endif /* GJ_MODEL_H */
