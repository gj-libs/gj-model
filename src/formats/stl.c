#include "gj_model/gj_model.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

int is_little_endian(void) {
    uint16_t x = 1;
    return *((uint8_t*)&x) == 1;
}

struct __attribute__((packed)) triangle {
    float normal[3];
    float v1[3];
    float v2[3];
    float v3[3];
    uint16_t attr;
};

int stl_parse_binary(FILE *fptr, struct gjModel *model) {
    uint32_t triCount;
    if (fread(&triCount, sizeof(uint32_t), 1, fptr) != 1) {
        printf("Failed to read triangle count\n");
        return -1;
    }

    // triCount * 3 vertices * 3 floats per vertex (x,y,z)|(nx,ny,nz) * sizeof(float)
    size_t dataSize = triCount * 3 * 3 * sizeof(float);
    float *positionData = malloc(dataSize);
    float *normalData = malloc(dataSize);
    if (!positionData || !normalData) {
        printf("Out of memory!\n");
        free(positionData);
        free(normalData);
        return -1;
    }

    struct gjMesh mesh = {
        .positions = positionData,
        .normals = normalData,
        .vertexCount = triCount * 3
    };
    mesh.indexCount = mesh.vertexCount;
    mesh.indices = malloc(sizeof(unsigned int) * mesh.indexCount);
    for (int i = 0; i < mesh.indexCount; i++)
        mesh.indices[i] = i;
    mesh.materialIndex = -1;

    struct gjMesh *tmp = realloc(
        model->meshes,
        sizeof(struct gjMesh) * (model->meshCount + 1)
    );

    if (!tmp) {
        printf("realloc failed\n");
        return -1;
    }
    model->meshes = tmp;
    model->meshes[model->meshCount] = mesh;
    model->meshCount++;
    model->root.meshCount++;
    model->root.meshIndices = realloc(
        model->root.meshIndices,
        sizeof(int) * model->root.meshCount
    );
    model->root.meshIndices[model->root.meshCount - 1] = model->meshCount - 1;

    for (size_t i = 0; i < triCount; i++) {
        struct triangle tri;
        if (fread(&tri, sizeof(struct triangle), 1, fptr) != 1) {
            printf("Failed to parse stl triangle\n");
            free(positionData);
            free(normalData);
            return -1;
        }
        size_t base = i * 9; // 3 for xyz
        // v1
        positionData[base + 0] = tri.v1[0];
        positionData[base + 1] = tri.v1[1];
        positionData[base + 2] = tri.v1[2];
        normalData[base + 0] = tri.normal[0];
        normalData[base + 1] = tri.normal[1];
        normalData[base + 2] = tri.normal[2];

        // v2
        positionData[base + 3] = tri.v2[0];
        positionData[base + 4] = tri.v2[1];
        positionData[base + 5] = tri.v2[2];
        normalData[base + 3] = tri.normal[0];
        normalData[base + 4] = tri.normal[1];
        normalData[base + 5] = tri.normal[2];

        // v3
        positionData[base + 6] = tri.v3[0];
        positionData[base + 7] = tri.v3[1];
        positionData[base + 8] = tri.v3[2];
        normalData[base + 6] = tri.normal[0];
        normalData[base + 7] = tri.normal[1];
        normalData[base + 8] = tri.normal[2];
    }
    return 0;
}

int count_facets(FILE *fptr) {
    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), fptr)) {
        if (strstr(line, "endfacet")) {
            count++;
        }
    }
    rewind(fptr);
    return count;
}

int stl_parse_ascii(FILE *fptr, struct gjModel *model) {
    int triCount = count_facets(fptr);

    size_t dataSize = triCount * 3 * 3 * sizeof(float);
    float *positionData = malloc(dataSize);
    float *normalData = malloc(dataSize);
    if (!positionData || !normalData) {
        printf("Out of memory!\n");
        free(positionData);
        free(normalData);
        return -1;
    }

    struct gjMesh mesh = {
        .positions = positionData,
        .normals = normalData,
        .vertexCount = triCount * 3
    };
    mesh.indexCount = mesh.vertexCount;
    mesh.indices = malloc(sizeof(unsigned int) * mesh.indexCount);
    for (int i = 0; i < mesh.indexCount; i++)
        mesh.indices[i] = i;
    mesh.materialIndex = -1;

    struct gjMesh *tmp = realloc(
        model->meshes,
        sizeof(struct gjMesh) * (model->meshCount + 1)
    );
    if (!tmp) return -1;
    model->meshes = tmp;
    model->meshes[model->meshCount] = mesh;
    model->meshCount++;
    model->root.meshCount++;
    model->root.meshIndices = realloc(
        model->root.meshIndices,
        sizeof(int) * model->root.meshCount
    );
    model->root.meshIndices[model->root.meshCount - 1] = model->meshCount - 1;

    char line[256];
    float x, y, z;
    int vCount = 0;
    int currentTriCount = 0;
    struct triangle tri = {0};
    while (fgets(line, sizeof(line), fptr)) {
        if (strstr(line, "facet normal")) {
            sscanf(line, "%*s %*s %f %f %f", &x, &y, &z);
            tri.normal[0] = x;
            tri.normal[1] = y;
            tri.normal[2] = z;
        } else if (strstr(line, "vertex")) {
            sscanf(line, "%*s %f %f %f", &x, &y, &z);
            if (vCount >= 3) {
                printf("Invalid triangle\n");
                continue;
            }
            if (vCount == 0) {
                tri.v1[0] = x;
                tri.v1[1] = y;
                tri.v1[2] = z;
            } else if (vCount == 1) {
                tri.v2[0] = x;
                tri.v2[1] = y;
                tri.v2[2] = z;
            } else if (vCount == 2) {
                tri.v3[0] = x;
                tri.v3[1] = y;
                tri.v3[2] = z;
            }
            vCount += 1;
        } else if (strstr(line, "endloop")) {
            size_t base = currentTriCount * 9; // 3 for xyz
            // v1
            positionData[base + 0] = tri.v1[0];
            positionData[base + 1] = tri.v1[1];
            positionData[base + 2] = tri.v1[2];
            normalData[base + 0] = tri.normal[0];
            normalData[base + 1] = tri.normal[1];
            normalData[base + 2] = tri.normal[2];

            // v2
            positionData[base + 3] = tri.v2[0];
            positionData[base + 4] = tri.v2[1];
            positionData[base + 5] = tri.v2[2];
            normalData[base + 3] = tri.normal[0];
            normalData[base + 4] = tri.normal[1];
            normalData[base + 5] = tri.normal[2];

            // v3
            positionData[base + 6] = tri.v3[0];
            positionData[base + 7] = tri.v3[1];
            positionData[base + 8] = tri.v3[2];
            normalData[base + 6] = tri.normal[0];
            normalData[base + 7] = tri.normal[1];
            normalData[base + 8] = tri.normal[2];

            currentTriCount++;
            vCount = 0;
        }
    }
    return 0;
}

int stl_parse_header(FILE *fptr) {
    // Read first 5 bytes, check if it starts with "solid"
    // solid -> ASCII STL file
    char asciiId[5];
    if (fread(asciiId, 5, 1, fptr) != 1) {
        printf("Failed to parse stl file header\n");
        // gj_set_error("Failed to parse stl file header\n");
        return -1;
    }

    if (memcmp(asciiId, "solid", 5) == 0) {
        return 0;
    } else {
        // If it's not ASCII
        // then should be binary
        // so read|skip a binary header - 80 bytes
        fseek(fptr, 80, SEEK_SET);
        return 1;
    }
    return -1;
}

int stl_open(const char *filename, struct gjModel *model) {
    FILE *fptr;

    if (!(fptr = fopen(filename, "rb"))) {
        printf("Failed to stl_open file %s\n", filename);
        // gj_set_error("Failed to stl_open file %s\n", filename);
        return -1;
    }

    int res = stl_parse_header(fptr);
    if (res == -1) {
        fclose(fptr);
        return -1;
    } else if (res == 0) { // ASCII
        if (stl_parse_ascii(fptr, model)) {
            fclose(fptr);
            return -1;
        }
    } else if (res == 1) { // binary
        if (stl_parse_binary(fptr, model)) {
            fclose(fptr);
            return -1;
        }
    }
    fclose(fptr);
    return 0;
}
