#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gj_model/gj_model.h"
#include "formats/mtl.h"

// Temporary storage for raw OBJ data
typedef struct {
    float *positions;    // v
    float *texcoords;    // vt
    float *normals;      // vn
    int posCount;
    int texCount;
    int normCount;
} RawObjData;

typedef struct {
    int v, vt, vn;
} FaceVertex;

typedef struct {
    FaceVertex vertices[3];
} Face;


typedef struct {
    char name[256];
    Face *faces;
    int faceCount;
    int faceCapacity;
    int materialIndex;
} ObjGroup;

typedef struct {
    ObjGroup *groups;
    int groupCount;
    int groupCapacity;
    int currentGroup;
} GroupList;

// Parse a face line (handles all formats: v, v/vt, v/vt/vn, v//vn)
static Face parse_face(const char *line) {
    Face face = {0};
    int v[3] = {0}, vt[3] = {0}, vn[3] = {0};
    int read;

    // Try: f v/vt/vn v/vt/vn v/vt/vn
    read = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                  &v[0], &vt[0], &vn[0],
                  &v[1], &vt[1], &vn[1],
                  &v[2], &vt[2], &vn[2]);
    if (read == 9) goto success;

    // Try: f v//vn v//vn v//vn
    read = sscanf(line, "f %d//%d %d//%d %d//%d",
                  &v[0], &vn[0], &v[1], &vn[1], &v[2], &vn[2]);
    if (read == 6) goto success;

    // Try: f v/vt v/vt v/vt
    read = sscanf(line, "f %d/%d %d/%d %d/%d",
                  &v[0], &vt[0], &v[1], &vt[1], &v[2], &vt[2]);
    if (read == 6) goto success;

    // Try: f v v v
    read = sscanf(line, "f %d %d %d", &v[0], &v[1], &v[2]);
    if (read == 3) goto success;

    fprintf(stderr, "Invalid face line: %s", line);
    return face;

success:
    for (int i = 0; i < 3; i++) {
        face.vertices[i].v = v[i];
        face.vertices[i].vt = vt[i];
        face.vertices[i].vn = vn[i];
    }
    return face;
}

// Initialize group list
static void init_group_list(GroupList *list) {
    list->groupCapacity = 8;
    list->groups = malloc(list->groupCapacity * sizeof(ObjGroup));
    list->groupCount = 0;
    list->currentGroup = -1;
}

// Add a new group
static void add_group(GroupList *list, const char *name) {
    if (list->groupCount >= list->groupCapacity) {
        list->groupCapacity *= 2;
        ObjGroup *tmp = realloc(list->groups, list->groupCapacity * sizeof(ObjGroup));
        if (!tmp) {
            fprintf(stderr, "Out of memory\n");
            return;
        }
        list->groups = tmp;
    }

    ObjGroup *group = &list->groups[list->groupCount];
    memset(group, 0, sizeof(ObjGroup));
    strncpy(group->name, name ? name : "default", 255);
    group->name[255] = '\0';
    group->faceCapacity = 64;
    group->faces = malloc(group->faceCapacity * sizeof(Face));
    group->faceCount = 0;
    group->materialIndex = -1;

    list->currentGroup = list->groupCount;
    list->groupCount++;
}

// Add face to current group
static void add_face(GroupList *list, Face face) {
    if (list->currentGroup < 0) {
        add_group(list, "default");
    }

    ObjGroup *group = &list->groups[list->currentGroup];

    if (group->faceCount >= group->faceCapacity) {
        group->faceCapacity *= 2;
        Face *tmp = realloc(group->faces, group->faceCapacity * sizeof(Face));
        if (!tmp) {
            fprintf(stderr, "Out of memory\n");
            return;
        }
        group->faces = tmp;
    }

    group->faces[group->faceCount++] = face;
}

// Find material index by name
static int find_material_index(struct gjMaterial *materials, int materialCount, const char *name) {
    for (int i = 0; i < materialCount; i++) {
        if (strcmp(materials[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Count elements in the OBJ file (for pre-allocation)
static void count_elements(FILE *fptr, int *vCount, int *vtCount, int *vnCount) {
    char line[256];
    *vCount = *vtCount = *vnCount = 0;

    while (fgets(line, sizeof(line), fptr)) {
        if (strncmp(line, "v ", 2) == 0) (*vCount)++;
        else if (strncmp(line, "vt ", 3) == 0) (*vtCount)++;
        else if (strncmp(line, "vn ", 3) == 0) (*vnCount)++;
    }
    rewind(fptr);
}

// Get directory path from filename
static void get_directory(const char *filepath, char *dirpath, size_t size) {
    const char *lastSlash = strrchr(filepath, '/');
    const char *lastBackslash = strrchr(filepath, '\\');
    const char *separator = lastSlash > lastBackslash ? lastSlash : lastBackslash;

    if (separator) {
        size_t len = separator - filepath + 1;
        if (len >= size) len = size - 1;
        strncpy(dirpath, filepath, len);
        dirpath[len] = '\0';
    } else {
        dirpath[0] = '\0';
    }
}

static int parse_obj_file(FILE *fptr, const char *objPath, RawObjData *raw, GroupList *groups,
                          struct gjMaterial **materials, int *materialCount) {
    int vCount = 0, vtCount = 0, vnCount = 0;
    count_elements(fptr, &vCount, &vtCount, &vnCount);

    // Allocate raw data arrays
    raw->positions = malloc(vCount * 3 * sizeof(float));
    raw->texcoords = vtCount > 0 ? malloc(vtCount * 2 * sizeof(float)) : NULL;
    raw->normals = vnCount > 0 ? malloc(vnCount * 3 * sizeof(float)) : NULL;
    raw->posCount = raw->texCount = raw->normCount = 0;

    if (!raw->positions || (vtCount > 0 && !raw->texcoords) || 
        (vnCount > 0 && !raw->normals)) {
        return -1;
    }

    init_group_list(groups);
    *materials = NULL;
    *materialCount = 0;

    char line[256];
    char dirpath[512];
    get_directory(objPath, dirpath, sizeof(dirpath));

    while (fgets(line, sizeof(line), fptr)) {
        float x, y, z, w;

        if (strncmp(line, "v ", 2) == 0) {
            w = 1.0f;
            int read = sscanf(line, "v %f %f %f %f", &x, &y, &z, &w);
            if (read >= 3) {
                raw->positions[raw->posCount * 3 + 0] = x;
                raw->positions[raw->posCount * 3 + 1] = y;
                raw->positions[raw->posCount * 3 + 2] = z;
                raw->posCount++;
            }
        } else if (strncmp(line, "vt ", 3) == 0) {
            z = 0.0f;
            int read = sscanf(line, "vt %f %f %f", &x, &y, &z);
            if (read >= 2) {
                raw->texcoords[raw->texCount * 2 + 0] = x;
                raw->texcoords[raw->texCount * 2 + 1] = y;
                raw->texCount++;
            }
        } else if (strncmp(line, "vn ", 3) == 0) {
            if (sscanf(line, "vn %f %f %f", &x, &y, &z) == 3) {
                raw->normals[raw->normCount * 3 + 0] = x;
                raw->normals[raw->normCount * 3 + 1] = y;
                raw->normals[raw->normCount * 3 + 2] = z;
                raw->normCount++;
            }
        } else if (strncmp(line, "f ", 2) == 0) {
            Face face = parse_face(line);
            add_face(groups, face);
        } else if (strncmp(line, "g ", 2) == 0 || strncmp(line, "o ", 2) == 0) {
            // New group/object
            char name[256] = {0};
            sscanf(line + 2, "%255s", name);
            add_group(groups, name);
        } else if (strncmp(line, "usemtl ", 7) == 0) {
            if (groups->currentGroup < 0) {
                add_group(groups, "default");
            }
            // Use material
            char mtlName[256] = {0};
            sscanf(line, "usemtl %255s", mtlName);
            if (groups->currentGroup >= 0 && *materials) {
                int mtlIdx = find_material_index(*materials, *materialCount, mtlName);
                groups->groups[groups->currentGroup].materialIndex = mtlIdx;
            }
        } else if (strncmp(line, "mtllib ", 7) == 0) {
            // Load material library
            char mtlFile[256] = {0};
            sscanf(line, "mtllib %255s", mtlFile);

            // Construct full path
            char mtlPath[768];
            snprintf(mtlPath, sizeof(mtlPath), "%s%s", dirpath, mtlFile);

            // Free existing materials if any
            if (*materials) {
                free(*materials);
            }

            *materials = mtl_open(mtlPath, materialCount);
            if (!*materials) {
                fprintf(stderr, "Warning: Failed to load MTL file: %s\n", mtlPath);
                *materialCount = 0;
            }
        }
    }

    // If no groups were created, create a default one with all faces
    if (groups->groupCount == 0) {
        add_group(groups, "default");
    }

    return 0;
}

// Convert indexed face data to flat vertex arrays for a single group
static struct gjMesh* build_mesh(RawObjData *raw, ObjGroup *group) {
    struct gjMesh *mesh = calloc(1, sizeof(struct gjMesh));
    if (!mesh) return NULL;

    int vertexCount = group->faceCount * 3;
    if (vertexCount == 0) {
        free(mesh);
        return NULL;
    }

    mesh->positions = malloc(vertexCount * 3 * sizeof(float));
    mesh->normals = raw->normals ? malloc(vertexCount * 3 * sizeof(float)) : NULL;
    mesh->texcoords = raw->texcoords ? malloc(vertexCount * 2 * sizeof(float)) : NULL;
    mesh->indices = malloc(vertexCount * sizeof(unsigned int));

    if (!mesh->positions || !mesh->indices) {
        free(mesh->positions);
        free(mesh->normals);
        free(mesh->texcoords);
        free(mesh->indices);
        free(mesh);
        return NULL;
    }

    memset(mesh->positions, 0, vertexCount * 3 * sizeof(float));
    if (mesh->normals) memset(mesh->normals, 0, vertexCount * 3 * sizeof(float));
    if (mesh->texcoords) memset(mesh->texcoords, 0, vertexCount * 2 * sizeof(float));

    int idx = 0;
    for (int i = 0; i < group->faceCount; i++) {
        for (int j = 0; j < 3; j++) {
            FaceVertex fv = group->faces[i].vertices[j];

            // Position (OBJ indices are 1-based)
            if (fv.v > 0 && fv.v <= raw->posCount) {
                int vi = (fv.v - 1) * 3;
                mesh->positions[idx * 3 + 0] = raw->positions[vi + 0];
                mesh->positions[idx * 3 + 1] = raw->positions[vi + 1];
                mesh->positions[idx * 3 + 2] = raw->positions[vi + 2];
            }

            // Normal
            if (mesh->normals && fv.vn > 0 && fv.vn <= raw->normCount) {
                int ni = (fv.vn - 1) * 3;
                mesh->normals[idx * 3 + 0] = raw->normals[ni + 0];
                mesh->normals[idx * 3 + 1] = raw->normals[ni + 1];
                mesh->normals[idx * 3 + 2] = raw->normals[ni + 2];
            }

            // Texcoord
            if (mesh->texcoords && fv.vt > 0 && fv.vt <= raw->texCount) {
                int ti = (fv.vt - 1) * 2;
                mesh->texcoords[idx * 2 + 0] = raw->texcoords[ti + 0];
                mesh->texcoords[idx * 2 + 1] = raw->texcoords[ti + 1];
            }

            mesh->indices[idx] = idx;
            idx++;
        }
    }

    mesh->vertexCount = vertexCount;
    mesh->indexCount = vertexCount;
    mesh->materialIndex = group->materialIndex;

    return mesh;
}

int obj_open(const char *filename, struct gjModel *model) {
    FILE *fptr = fopen(filename, "r");
    if (!fptr) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return -1;
    }

    RawObjData raw = {0};
    GroupList groups = {0};
    struct gjMaterial *materials = NULL;
    int materialCount = 0;

    if (parse_obj_file(fptr, filename, &raw, &groups, &materials, &materialCount) != 0) {
        fclose(fptr);
        free(raw.positions);
        free(raw.texcoords);
        free(raw.normals);
        return -1;
    }
    fclose(fptr);

    // Build meshes from groups
    struct gjMesh *meshes = malloc(groups.groupCount * sizeof(struct gjMesh));
    if (!meshes) {
        free(raw.positions);
        free(raw.texcoords);
        free(raw.normals);
        for (int i = 0; i < groups.groupCount; i++) {
            free(groups.groups[i].faces);
        }
        free(groups.groups);
        free(materials);
        return -1;
    }

    int validMeshCount = 0;
    for (int i = 0; i < groups.groupCount; i++) {
        struct gjMesh *mesh = build_mesh(&raw, &groups.groups[i]);
        if (mesh) {
            meshes[validMeshCount++] = *mesh;
            free(mesh);
        }
        free(groups.groups[i].faces);
    }

    // Clean up temporary data
    free(raw.positions);
    free(raw.texcoords);
    free(raw.normals);
    free(groups.groups);

    if (validMeshCount == 0) {
        free(meshes);
        free(materials);
        return -1;
    }

    model->meshes = meshes;
    model->meshCount = validMeshCount;
    model->materials = materials;
    model->materialCount = materialCount;

    // Initialize root node with identity transform
    model->root.transform[0] = 1;
    model->root.transform[5] = 1;
    model->root.transform[10] = 1;
    model->root.transform[15] = 1;

    // Assign all meshes to root node
    model->root.meshIndices = malloc(validMeshCount * sizeof(int));
    for (int i = 0; i < validMeshCount; i++) {
        model->root.meshIndices[i] = i;
    }
    model->root.meshCount = validMeshCount;
    model->root.children = NULL;
    model->root.childCount = 0;

    return 0;
}
