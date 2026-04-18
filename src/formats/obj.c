#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gj_model/gj_model.h"

struct faceVertex {
    int v;
    int vt;
    int vn;
};

struct objData {
    float *positions; // v  (xyz)
    float *normals;   // vn (xyz)
    float *texcoords; // vt (uvw)

    int positionCount;
    int normalCount;
    int texcoordsCount;

    struct faceVertex *faces; // f (v/vt/vn)*3
    int faceCount; // number of vertices (not triangles)
};

void parse_line(const char *line, struct objData *objData) {
    float x = 0, y = 0, z = 0, w = 1.0f;
    int read;
    if (strncmp(line, "v ", 2) == 0) {
        // add rgb
        w = 1.0f;
        read = sscanf(line, "v %f %f %f %f", &x, &y, &z, &w);
        if (read < 3) {
            printf("Invalid v line\n");
        }
        objData->positions[objData->positionCount*3+0] = x/w;
        objData->positions[objData->positionCount*3+1] = y/w;
        objData->positions[objData->positionCount*3+2] = z/w;
        objData->positionCount++;
    } else if (strncmp(line, "vt ", 3) == 0) {
        y = 0.0f;
        z = 0.0f;
        read = sscanf(line, "vt %f %f %f", &x, &y, &z);
        if (read < 2) {
            printf("Invalid vt line\n");
        }
        objData->texcoords[objData->texcoordsCount*3+0] = x;
        objData->texcoords[objData->texcoordsCount*3+1] = y;
        objData->texcoords[objData->texcoordsCount*3+2] = z;
        objData->texcoordsCount++;
    } else if (strncmp(line, "vn ", 3) == 0) {
        read = sscanf(line, "vn %f %f %f", &x, &y, &z);
        if (read != 3) {
            printf("Invalid vn line\n");
        }
        objData->normals[objData->normalCount*3+0] = x;
        objData->normals[objData->normalCount*3+1] = y;
        objData->normals[objData->normalCount*3+2] = z;
        objData->normalCount++;
    } else if (strncmp(line, "vp ", 3) == 0) {
        read = sscanf(line, "vp %f %f %f", &x, &y, &z);
    } else if (strncmp(line, "f ", 2) == 0) {
        int v[3]={0}, vt[3]={0}, vn[3]={0};
        int read;
        int readValidLine = 0;

        // f v/vt/vn v/vt/vn v/vt/vn
        if (!readValidLine) {
            read = sscanf(line,
                          "f %d/%d/%d %d/%d/%d %d/%d/%d",
                          &v[0], &vt[0], &vn[0],
                          &v[1], &vt[1], &vn[1],
                          &v[2], &vt[2], &vn[2]
                          );
            if (read == 9) {
                readValidLine = 1;
            }
        }

        // f v//vn v//vn v//vn
        if (!readValidLine) {
            read = sscanf(line,
                          "f %d//%d %d//%d %d//%d",
                          &v[0], &vn[0],
                          &v[1], &vn[1],
                          &v[2], &vn[2]
                          );
            if (read == 6) {
                readValidLine = 1;
            }
        }

        // f v/vt v/vt v/vt
        if (!readValidLine) {
            read = sscanf(line,
                          "f %d/%d %d/%d %d/%d",
                          &v[0], &vt[0],
                          &v[1], &vt[1],
                          &v[2], &vt[2]
                          );
            if (read == 6) {
                readValidLine = 1;
            }
        }

        // f v v v
        if (!readValidLine) {
            read = sscanf(line,
                          "f %d %d %d",
                          &v[0],
                          &v[1],
                          &v[2]
                          );
            if (read == 3) {
                readValidLine = 1;
            }
        }

        if (!readValidLine) {
            printf("Invalid face line: %s\n", line);
            return;
        }
        for (int i = 0; i < 3; i++) {
            objData->faces[objData->faceCount + i].v  = v[i];
            objData->faces[objData->faceCount + i].vt = vt[i];
            objData->faces[objData->faceCount + i].vn = vn[i];
        }
        objData->faceCount += 3;
    }
}

struct Mesh obj_open(const char *filename) {
    FILE *fptr;

    if (!(fptr = fopen(filename, "rb"))) {
        printf("Failed to stl_open file %s\n", filename);
        struct Mesh mesh = {0};
        return mesh;
    }
#define MAX_VERTS 36
#define MAX_FACES 36

    struct objData objData = {0};
    objData.positions = malloc(MAX_VERTS * 3 * sizeof(float));
    objData.normals   = malloc(MAX_VERTS * 3 * sizeof(float));
    objData.texcoords = malloc(MAX_VERTS * 3 * sizeof(float));

    objData.faces     = malloc(MAX_FACES * 3 * sizeof(struct faceVertex));

    char line[256];
    while (fgets(line, sizeof(line), fptr)) {
        parse_line(line, &objData);
    }

    int hasNormals = objData.normalCount > 0;
    int hasTexcoords = objData.texcoordsCount > 0;
    int stride = 8; // floats per vertex
    float *data = malloc(sizeof(float) * objData.faceCount * stride);

    struct Mesh mesh = {
        .vertices     = data,
        .vertexCount  = objData.faceCount,
        .hasNormals   = hasNormals,
        .hasTexcoords = hasTexcoords
    };

    int dataOffset = 0;
    for (int i = 0; i < objData.faceCount; i++) {
        struct faceVertex fv = objData.faces[i];

        float *pos  = &objData.positions[(fv.v - 1) * 3];
        float *norm = &objData.normals[(fv.vn - 1) * 3];
        float *uv = &objData.texcoords[(fv.vt - 1) * 3];

        data[dataOffset+0] = pos[0]; // x
        data[dataOffset+1] = pos[1]; // y
        data[dataOffset+2] = pos[2]; // z

        data[dataOffset+3] = norm[0]; // x
        data[dataOffset+4] = norm[1]; // y
        data[dataOffset+5] = norm[2]; // z

        data[dataOffset+6] = uv[0]; // u
        data[dataOffset+7] = uv[1]; // v

        dataOffset += stride;
    }
    return mesh;
}
