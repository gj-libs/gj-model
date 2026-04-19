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

struct Mesh {
    float *vertices;
    int nVertices;

    int hasNormals;
    int hasTexcoords;
};

/*
 * Opens a 3D model file
 *
 * @param filename Path to the model file
 *
 * @return A struct Mesh
 *
 * Mesh.vertices has 24 floats per vertice
 * in the format (x,y,z,nx,ny,nz,u,v)
 *   x,y,z    are position coords
 *   nx,ny,nz are normals
 *   u,v      are texture coords
 *
 * The returned Mesh.vertices buffer must be freed using gj_model_free().
 */
struct Mesh gj_model_load(const char *filename);

/*
 * Frees the allocated image data
 *
 * @param data Pointer to the image data to be freed
 */
void        gj_model_free(struct Mesh *mesh);

#ifdef __cplusplus
}
#endif

#endif /* GJ_MODEL_H */
