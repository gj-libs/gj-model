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

struct Mesh stl_parse_binary(FILE *fptr) {
    uint32_t triCount;
    struct Mesh nullMesh = {0};
    if (fread(&triCount, sizeof(uint32_t), 1, fptr) != 1) {
        printf("Failed to read triangle count\n");
        return nullMesh;
    }

    // triCount * 3 vertices * 8 floats per vertex (x,y,z,nx,ny,nz,u,v) * sizeof(float)
    //
    size_t dataSize = triCount * 3 * 8 * sizeof(float);
    float *data = malloc(dataSize);
    if (!data) {
        printf("Failed to allocate memory ofr stl data\n");
        return nullMesh;
    }

    struct Mesh mesh = {
        .vertices     = data,
        .vertexCount  = triCount * 3, // 3 = vertix per triangle
        .hasNormals   = 1,
        .hasTexcoords = 0
    };

    for (size_t i = 0; i < triCount; i++) {
        struct triangle tri;
        if (fread(&tri, sizeof(struct triangle), 1, fptr) != 1) {
            printf("Failed to parse stl triangle\n");
            free(data);
            return nullMesh;
        }
        size_t base = i * 24; // 3 * 8
        // v1
        data[base + 0] = tri.v1[0];
        data[base + 1] = tri.v1[1];
        data[base + 2] = tri.v1[2];
        data[base + 3] = tri.normal[0];
        data[base + 4] = tri.normal[1];
        data[base + 5] = tri.normal[2];
        data[base + 6] = 0; // u
        data[base + 7] = 0; // v

        // v2
        data[base + 8] = tri.v2[0];
        data[base + 9] = tri.v2[1];
        data[base + 10] = tri.v2[2];
        data[base + 11] = tri.normal[0];
        data[base + 12] = tri.normal[1];
        data[base + 13] = tri.normal[2];
        data[base + 14] = 0; // u
        data[base + 15] = 0; // v

        // v3
        data[base + 16] = tri.v3[0];
        data[base + 17] = tri.v3[1];
        data[base + 18] = tri.v3[2];
        data[base + 19] = tri.normal[0];
        data[base + 20] = tri.normal[1];
        data[base + 21] = tri.normal[2];
        data[base + 22] = 0; // u
        data[base + 23] = 0; // v
    }
    return mesh;
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

struct Mesh stl_parse_ascii(FILE *fptr) {
    int triCount = count_facets(fptr);

    size_t dataSize = triCount * 3 * 8 * sizeof(float);
    float *data = malloc(dataSize);
    if (!data) {
        printf("Failed to allocate memory ofr stl data\n");
        struct Mesh nullMesh = {0};
        return nullMesh;
    }

    struct Mesh mesh = {
        .vertices     = data,
        .vertexCount  = triCount * 3, // 3 = vertix per triangle
        .hasNormals   = 1,
        .hasTexcoords = 0
    };

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
            size_t base = currentTriCount * 24; // 3 * 8
            // v1
            data[base + 0] = tri.v1[0];
            data[base + 1] = tri.v1[1];
            data[base + 2] = tri.v1[2];
            data[base + 3] = tri.normal[0];
            data[base + 4] = tri.normal[1];
            data[base + 5] = tri.normal[2];
            data[base + 6] = 0; // u
            data[base + 7] = 0; // v

            // v2
            data[base + 8] = tri.v2[0];
            data[base + 9] = tri.v2[1];
            data[base + 10] = tri.v2[2];
            data[base + 11] = tri.normal[0];
            data[base + 12] = tri.normal[1];
            data[base + 13] = tri.normal[2];
            data[base + 14] = 0; // u
            data[base + 15] = 0; // v

            // v3
            data[base + 16] = tri.v3[0];
            data[base + 17] = tri.v3[1];
            data[base + 18] = tri.v3[2];
            data[base + 19] = tri.normal[0];
            data[base + 20] = tri.normal[1];
            data[base + 21] = tri.normal[2];
            data[base + 22] = 0; // u
            data[base + 23] = 0; // v

            currentTriCount++;
            vCount = 0;
        }
    }
    return mesh;
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

struct Mesh stl_open(const char *filename) {
    FILE *fptr;

    struct Mesh nullMesh = {0};
    if (!(fptr = fopen(filename, "rb"))) {
        printf("Failed to stl_open file %s\n", filename);
        // gj_set_error("Failed to stl_open file %s\n", filename);
        return nullMesh;
    }

    int res = stl_parse_header(fptr);
    if (res == -1) {
        fclose(fptr);
        return nullMesh;
    } else if (res == 0) { // ASCII
        return stl_parse_ascii(fptr);
    } else if (res == 1) { // binary
        return stl_parse_binary(fptr);
    }
    return nullMesh;
}
