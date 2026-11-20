#include "formats/gltf.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PARSE_STRING(fptr, field) \
    do { \
        field = parse_string(fptr); \
        if (!field) { \
            free(key); \
            return -1; \
        } \
    } while (0)

#define PARSE_SIZE_T(fptr, field) \
    do { \
        if (parse_size_t(fptr, field)) { \
            free(key); \
            return -1; \
        } \
    } while (0)

#define FREE_ALL(...) \
    do { \
        void *ptrs[] = {__VA_ARGS__}; \
        for (size_t i = 0; i < sizeof(ptrs) / sizeof(ptrs[0]); i++) \
            free(ptrs[i]); \
    } while (0)

#define CONSUME(fptr, c, ...) \
    do { \
        if (fgetc(fptr) != (c)) { \
            FREE_ALL(__VA_ARGS__); \
            return -1; \
        } \
    } while (0)

struct asset {
    char *version;
    char *generator;
    char *copyright;
};

struct buffer {
    size_t byteLength;
    char *uri;
};

struct buffers {
    size_t count;
    struct buffer *buffers;
};

struct bufferView {
    size_t buffer;
    size_t byteOffset;
    size_t byteLength;
    size_t target;
};

struct bufferViews {
    size_t count;
    struct bufferView *bufferViews;
};

struct accessor {
    size_t bufferView;
    size_t byteOffset;
    size_t componentType;
    size_t count;
    char *max;
    char *min;
    char *type;
};

struct accessors {
    size_t count;
    struct accessor *accessors;
};

struct image {
    char *uri;
};

struct images {
    size_t count;
    struct image *images;
};

struct material {
    char *name;
    char *pbrMetallicRoughness;
    size_t baseColorTextureIndex;
    size_t metallicRoughnessTextureIndex;
};

struct materials {
    size_t count;
    struct material *materials;
};

struct attributes {
    size_t NORMAL;
    size_t POSITION;
    size_t TANGENT;
    size_t TEXCOORD_0;
};

struct primitive {
    struct attributes attributes;
    size_t indices;
    size_t material;
    size_t mode;
};

struct mesh {
    char *name;
    struct primitive *primitives;
    size_t primitiveCount;
};

struct meshes {
    size_t count;
    struct mesh *meshes;
};

struct node {
    char *name;
    size_t mesh;
};

struct nodes {
    size_t count;
    struct node *nodes;
};

struct sampler {
    char *name;
    size_t mesh;
};

struct samplers {
    size_t count;
    struct sampler *samplers;
};

struct scenes_node {
    size_t *nodes;
    size_t nodeCount;
};

struct scenes_nodes {
    size_t count;
    struct scenes_node *scenes_nodes;
};

struct scene {
    size_t count;
    struct scenes_node *scenes_nodes;
};

struct scenes {
    size_t count;
    struct scene *scenes;
};

void skip_whitespaces(FILE *fptr) {
    char c;
    while ((c = fgetc(fptr)) != EOF) {
        if (c != ' ' && c != '\n' && c != '\t' && c != '\r') {
            ungetc(c, fptr);
            break;
        }
    }
}

int get_string(FILE *fptr, char *buffer, size_t buffer_size) {
    skip_whitespaces(fptr);
    char c = fgetc(fptr);
    if (c != '"') {
        return -1;
    }
    size_t i = 0;
    while ((c = fgetc(fptr)) != EOF && c != '"') {
        if (i < buffer_size - 1) {
            buffer[i++] = c;
        }
    }
    buffer[i] = '\0';
    if (c != '"') {
        return -1;
    }
    return 0;
}

char *parse_string(FILE *fptr) {
    skip_whitespaces(fptr);
    char c = fgetc(fptr);
    if (c != '"')
        return NULL;

    size_t capacity = 16;
    size_t length = 0;
    char *str = malloc(capacity);

    if (!str)
        return NULL;

    while ((c = fgetc(fptr)) != EOF && c != '"') {
        if (length + 1 >= capacity) {
            capacity *= 2;
            char *new_str = realloc(str, capacity);
            if (!new_str) {
                free(str);
                return NULL;
            }
            str = new_str;
        }
        str[length++] = c;
    }
    str[length] = '\0';
    if (c != '"') {
        free(str);
        return NULL;
    }
    return str;
}

int parse_size_t(FILE *fptr, size_t *value) {
    skip_whitespaces(fptr);
    char c = fgetc(fptr);
    if (!(c >= '0' && c <= '9'))
        return -1;

    int r = 0;
    while (c >= '0' && c <= '9') {
        r = r * 10 + (c - '0');
        c = fgetc(fptr);
    }

    ungetc(c, fptr);
    *value = r;
    return 0;
}

char *parse_key(FILE *fptr) {
    skip_whitespaces(fptr);
    char c = fgetc(fptr);
    if (c == EOF)
        return NULL;
    // TODO: ????
    // if (c == '}')
    //     return 0;
    if (c != '"')
        return NULL;
    ungetc(c, fptr);
    return parse_string(fptr);
}

int parse_object_array(
        FILE *fptr,
        void **array,
        size_t *count,
        size_t elem_size,
        int (*parse_func)(FILE *, void *)
) {
    size_t capacity = 4;
    *array = malloc(capacity * elem_size);
    if (!*array) {
        return -1;
    }
    *count = 0;
    while (1) {
        skip_whitespaces(fptr);
        char c = fgetc(fptr);
        if (c != '{') {
            free(*array);
            return -1;
        }
        if (*count >= capacity) {
            capacity *= 2;
            void *new_array = realloc(*array, capacity * elem_size);
            if (!new_array) {
                free(*array);
                return -1;
            }
            *array = new_array;
        }
        if (parse_func(fptr, (char *)(*array) + (*count) * elem_size)) {
            free(*array);
            return -1;
        }
        (*count)++;
        skip_whitespaces(fptr);
        c = fgetc(fptr);
        if (c == ',') {
            continue;
        } else if (c == ']') {
            break;
        } else {
            free(*array);
            return -1;
        }
    }
    return 0;
}

int parse_asset(FILE *fptr, struct asset *ad) {
    char *key = parse_key(fptr);
    if (!key)
        return -1;

    skip_whitespaces(fptr);
    if (fgetc(fptr) != ':')
        return -1;

    skip_whitespaces(fptr);
    if (strcmp(key, "version") == 0) 
        PARSE_STRING(fptr, ad->version);
    else if (strcmp(key, "generator") == 0) 
        PARSE_STRING(fptr, ad->generator);
    else if (strcmp(key, "copyright") == 0) 
        PARSE_STRING(fptr, ad->copyright);
    free(key);

    skip_whitespaces(fptr);
    char c = fgetc(fptr);
    if (c == ',') 
        // FIX: This can leak memory if the object contains same key multiple times
        return parse_asset(fptr, ad);
    else if (c == '}') 
        return 0;
    else 
        return -1;
}

int parse_accessor(FILE *fptr, struct accessor *ac) {
    char type_min_max_seen = 0;
    unsigned long max_position;
    unsigned long min_position;
    while (1) {
        char *key = parse_key(fptr);
        if (!key)
            return -1;

        skip_whitespaces(fptr);
        if (getc(fptr) != ':')
            return -1;

        skip_whitespaces(fptr);
        if (strcmp(key, "bufferView") == 0) {
            PARSE_SIZE_T(fptr, &ac->bufferView);
        } else if (strcmp(key, "byteOffset") == 0) {
            PARSE_SIZE_T(fptr, &ac->byteOffset);
        } else if (strcmp(key, "componentType") == 0) {
            PARSE_SIZE_T(fptr, &ac->componentType);
        } else if (strcmp(key, "count") == 0) {
            PARSE_SIZE_T(fptr, &ac->count);
        } else if (strcmp(key, "max") == 0) {
            char c;
            if (fgetc(fptr) != '[') {
                free(key);
                return -1;
            }
            max_position = ftell(fptr);
            while ((c = fgetc(fptr)) != ']') { }
            type_min_max_seen |= 0b001;
        } else if (strcmp(key, "min") == 0) {
            char c;
            if (fgetc(fptr) != '[') {
                free(key);
                return -1;
            }
            min_position = ftell(fptr);
            while ((c = fgetc(fptr)) != ']') { }
            type_min_max_seen |= 0b010;
        } else if (strcmp(key, "type") == 0) {
            PARSE_STRING(fptr, ac->type);
            type_min_max_seen |= 0b100;
        }

        // Ignore min/max/type for now.
        /*
        if (type_min_max_seen == 0b111) {
            if (!ac->type) {
                free(key);
                return -1;
            }

            fseek(fptr, max_position, SEEK_SET);
            if (parse_num_array(fptr, &ac->max, ac->type)) {
                free(key);
                return -1;
            }

            fseek(fptr, min_position, SEEK_SET);
            if (parse_num_array(fptr, &ac->max, ac->type)) {
                free(key);
                return -1;
            }
        }
        */

        skip_whitespaces(fptr);
        char c = fgetc(fptr);
        if (c == ',') {
            free(key);
            continue;
        } else if (c == '}') {
            free(key);
            break;
        } else {
            free(key);
            return -1;
        }
    }
    return 0;
}

int parse_bufferView(FILE *fptr, struct bufferView *bv) {
    while (1) {
        char *key = parse_key(fptr);
        if (!key)
            return -1;

        skip_whitespaces(fptr);
        if (+getc(fptr) != ':')
            return -1;

        skip_whitespaces(fptr);
        if (strcmp(key, "buffer") == 0) {
            PARSE_SIZE_T(fptr, &bv->buffer);
        } else if (strcmp(key, "byteLength") == 0) {
            PARSE_SIZE_T(fptr, &bv->byteLength);
        } else if (strcmp(key, "byteOffset") == 0) {
            PARSE_SIZE_T(fptr, &bv->byteOffset);
        } else if (strcmp(key, "target") == 0) {
            PARSE_SIZE_T(fptr, &bv->target);
        }

        skip_whitespaces(fptr);
        char c = fgetc(fptr);
        if (c == ',') {
            free(key);
            continue;
        } else if (c == '}') {
            free(key);
            break;
        } else {
            free(key);
            return -1;
        }
    }
    return 0;
}

int parse_buffer(FILE *fptr, struct buffer *b) {
    while (1) {
        char *key = parse_key(fptr);
        if (!key)
            return -1;

        skip_whitespaces(fptr);
        if (+getc(fptr) != ':')
            return -1;

        skip_whitespaces(fptr);
        if (strcmp(key, "byteLength") == 0) {
            PARSE_SIZE_T(fptr, &b->byteLength);
        } else if (strcmp(key, "uri") == 0) {
            PARSE_STRING(fptr, b->uri);
        }

        skip_whitespaces(fptr);
        char c = fgetc(fptr);
        if (c == ',') {
            free(key);
            continue;
        } else if (c == '}') {
            free(key);
            break;
        } else {
            free(key);
            return -1;
        }
    }
    return 0;
}

int parse_image(FILE *fptr, struct image *im) {
    char *key = parse_key(fptr);
    if (!key)
        return -1;

    skip_whitespaces(fptr);
    if (fgetc(fptr) != ':')
        return -1;

    skip_whitespaces(fptr);
    if (strcmp(key, "uri") == 0) 
        PARSE_STRING(fptr, im->uri);
    free(key);

    skip_whitespaces(fptr);
    char c = fgetc(fptr);
    if (c == ',') 
        // FIX: This can leak memory if the object contains same key multiple times
        return parse_image(fptr, im);
    else if (c == '}') 
        return 0;
    else 
        return -1;
}

int parse_material(FILE *fptr, struct material *ma) {
    while (1) {
        skip_whitespaces(fptr);
        char *key = parse_key(fptr);
        if (!key)
            return -1;

        skip_whitespaces(fptr);
        CONSUME(fptr, ':');

        skip_whitespaces(fptr);
        if (strcmp(key, "name") == 0) {
            PARSE_STRING(fptr, ma->name);
        } else if (strcmp(key, "pbrMetallicRoughness") == 0) {
            CONSUME(fptr, '{', key);

            while (1) {
                skip_whitespaces(fptr);
                char *key1 = parse_key(fptr);
                if (!key1) {
                    free(key);
                    return -1;
                }

                if (strcmp(key1, "baseColorTexture") == 0) {
                    skip_whitespaces(fptr);
                    CONSUME(fptr, ':', key1, key);
                    skip_whitespaces(fptr);
                    CONSUME(fptr, '{', key1, key);
                    skip_whitespaces(fptr);
                    char *key2 = parse_key(fptr);
                    if (!key2) {
                        free(key1);
                        free(key);
                        return -1;
                    }

                    skip_whitespaces(fptr);
                    CONSUME(fptr, ':', key2, key1, key);
                    if (strcmp(key2, "index") == 0) {
                        PARSE_SIZE_T(fptr, &ma->baseColorTextureIndex);
                    }
                } else if (strcmp(key1, "metallicRoughnessTexture") == 0) {
                    skip_whitespaces(fptr);
                    CONSUME(fptr, ':', key1, key);
                    skip_whitespaces(fptr);
                    CONSUME(fptr, '{', key1, key);
                    skip_whitespaces(fptr);
                    char *key2 = parse_key(fptr);
                    if (!key2) {
                        free(key1);
                        free(key);
                        return -1;
                    }

                    skip_whitespaces(fptr);
                    CONSUME(fptr, ':', key2, key1, key);
                    if (strcmp(key2, "index") == 0) {
                        PARSE_SIZE_T(fptr, &ma->metallicRoughnessTextureIndex);
                    }
                }

                skip_whitespaces(fptr);
                char c = fgetc(fptr);
                if (c == '}') {
                    c = fgetc(fptr);
                    if (c == ',')
                        continue;
                    else
                        break;
                } else {
                    return -1;
                }
            }
            skip_whitespaces(fptr);
            char c = fgetc(fptr);
            if (c != '}')
                return -1;
        }
        free(key);

        skip_whitespaces(fptr);
        char c = fgetc(fptr);
        if (c == ',') 
            continue;
        else if (c == '}') 
            break;
        else 
            return -1;
    }
    return 0;
}

int parse_primitive(FILE *fptr, struct primitive *p) {
    while (1) {
        skip_whitespaces(fptr);
        char *key = parse_key(fptr);
        if (!key)
            return -1;

        skip_whitespaces(fptr);
        CONSUME(fptr, ':', key);

        skip_whitespaces(fptr);
        if (strcmp(key, "attributes") == 0) {
            CONSUME(fptr, '{', key);
            while (1) {
                skip_whitespaces(fptr);
                char *key1 = parse_key(fptr);
                if (!key1) {
                    free(key);
                    return -1;
                }
                skip_whitespaces(fptr);
                CONSUME(fptr, ':', key1, key);
                skip_whitespaces(fptr);
                if (strcmp(key1, "POSITION") == 0) {
                    PARSE_SIZE_T(fptr, &p->attributes.POSITION);
                } else if (strcmp(key1, "NORMAL") == 0) {
                    PARSE_SIZE_T(fptr, &p->attributes.NORMAL);
                } else if (strcmp(key1, "TANGENT") == 0) {
                    PARSE_SIZE_T(fptr, &p->attributes.TANGENT);
                } else if (strcmp(key1, "TEXCOORD_0") == 0) {
                    PARSE_SIZE_T(fptr, &p->attributes.TEXCOORD_0);
                }
                skip_whitespaces(fptr);
                char c = fgetc(fptr);

                if (c == '}')
                    break;
                if (c == ',')
                    continue;
                else
                    return -1; // TODO: free
            }
        } else if (strcmp(key, "indices") == 0) {
            PARSE_SIZE_T(fptr, &p->indices);
        } else if (strcmp(key, "material") == 0) {
            PARSE_SIZE_T(fptr, &p->material);
        } else if (strcmp(key, "mode") == 0) {
            PARSE_SIZE_T(fptr, &p->mode);
        }
        free(key);

        skip_whitespaces(fptr);
        char c = fgetc(fptr);
        if (c == ',') 
            continue;
        else if (c == '}') 
            break;
        else 
            return -1;
    }
    return 0;
}

int parse_meshes(FILE *fptr, struct mesh *m) {
    while (1) {
        skip_whitespaces(fptr);
        char *key = parse_key(fptr);
        if (!key)
            return -1;

        skip_whitespaces(fptr);
        CONSUME(fptr, ':');

        skip_whitespaces(fptr);
        if (strcmp(key, "name") == 0) {
            PARSE_STRING(fptr, m->name);
        } else if (strcmp(key, "primitives") == 0) {
            CONSUME(fptr, '[', key);
            struct primitive primitives;
            if (parse_object_array(fptr, (void *)&primitives,
                        &m->primitiveCount, sizeof(struct primitive),
                        (int (*)(FILE *, void *))parse_primitive)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu primitives\n", m->primitiveCount);
        }
        free(key);

        skip_whitespaces(fptr);
        char c = fgetc(fptr);
        if (c == ',') 
            continue;
        else if (c == '}') 
            break;
        else 
            return -1;
    }
    return 0;
}

int parse_nodes(FILE *fptr, struct node *s) {
    while (1) {
        skip_whitespaces(fptr);
        char *key = parse_key(fptr);
        if (!key)
            return -1;

        skip_whitespaces(fptr);
        CONSUME(fptr, ':');

        skip_whitespaces(fptr);
        if (strcmp(key, "name") == 0) {
            PARSE_STRING(fptr, s->name);
        } else if (strcmp(key, "mesh") == 0) {
            PARSE_SIZE_T(fptr, &s->mesh);
        }
        free(key);

        skip_whitespaces(fptr);
        char c = fgetc(fptr);
        if (c == ',') 
            continue;
        else if (c == '}') 
            break;
        else 
            return -1;
    }
    return 0;
}

int parse_samplers(FILE *fptr, struct sampler *s) {
    skip_whitespaces(fptr);
    char c = fgetc(fptr);
    if (c == '}') 
        return 0;
    else 
        return -1;
    return 0;
}

int parse_scenes_nodes(FILE *fptr, struct scenes_nodes *s) {
    skip_whitespaces(fptr);
    char *key = parse_key(fptr);
    if (!key)
        return -1;

    skip_whitespaces(fptr);
    CONSUME(fptr, ':');

    skip_whitespaces(fptr);
    if (strcmp(key, "nodes") == 0) {
        CONSUME(fptr, '[', key);
        skip_whitespaces(fptr);
        size_t count = 0;
        size_t capacity = 4;
        s->scenes_nodes = malloc(capacity * sizeof(struct scenes_node));
        while (1) {
            if (count >= capacity) {
                capacity *= 2;
                struct scenes_node *new_nodes = realloc(
                        s->scenes_nodes, capacity * sizeof(struct scenes_node));
                if (!new_nodes) {
                    free(s->scenes_nodes);
                    return -1;
                }
                s->scenes_nodes = new_nodes;
            }
            PARSE_SIZE_T(fptr, &s->scenes_nodes[s->count].nodes[count]);
            count++;
            char c = fgetc(fptr);
            if (c == ',')
                continue;
            else if (c == ']')
              break;
            else
              return -1;
        }
        CONSUME(fptr, '}', key);
        printf("Parsed %zu scenes.nodes\n", count);
    }
    free(key);

    skip_whitespaces(fptr);
    char c = fgetc(fptr);
    if (c == '}') 
        return 0;
    else 
        return -1;
}

int parse_scenes(FILE *fptr, struct scene *s) {
    while (1) {
        skip_whitespaces(fptr);
        char *key = parse_key(fptr);
        if (!key)
            return -1;

        skip_whitespaces(fptr);
        CONSUME(fptr, ':');

        skip_whitespaces(fptr);
        if (strcmp(key, "nodes") == 0) {
            CONSUME(fptr, '[', key);
            struct scenes_nodes scenes_nodes;
            if (parse_object_array(fptr, (void *)&scenes_nodes,
                        &scenes_nodes.count, sizeof(struct scenes_nodes),
                        (int (*)(FILE *, void *))parse_scenes_nodes)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu scenes.nodes\n", scenes_nodes.count);
        }
        free(key);

        skip_whitespaces(fptr);
        char c = fgetc(fptr);
        if (c == ',') 
            continue;
        else if (c == '}') 
            break;
        else 
            return -1;
    }
    return 0;
}
int parse_gltf_object(FILE *fptr) {
    skip_whitespaces(fptr);
    int first = 1;
    while (1) {
        char c = fgetc(fptr);

        if (c == EOF)
            return -1;
        if (c == '}')
            return 0;

        if (c == ',') {
            if (first)
                return -1;
        } else {
            ungetc(c, fptr);
        }

        char *key = parse_string(fptr);
        if (!key)
            return -1;

        skip_whitespaces(fptr);
        if (fgetc(fptr) != ':')
            return -1;

        skip_whitespaces(fptr);
        if (strcmp(key, "asset") == 0) {
            c = fgetc(fptr);
            if (c != '{')
                return -1;
            struct asset asset = {0};
            if (parse_asset(fptr, &asset)) {
                free(key);
                return -1;
            }
        } else if (strcmp(key, "accessors") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct accessors accessors;
            if (parse_object_array(fptr, (void *)&accessors.accessors,
                        &accessors.count, sizeof(struct accessor),
                        (int (*)(FILE *, void *))parse_accessor)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu accessors\n", accessors.count);
        } else if (strcmp(key, "bufferViews") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct bufferViews bufferViews;
            if (parse_object_array(fptr, (void *)&bufferViews.bufferViews,
                        &bufferViews.count, sizeof(struct bufferView),
                        (int (*)(FILE *, void *))parse_bufferView)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu bufferViews\n", bufferViews.count);
        } else if (strcmp(key, "buffers") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct buffers buffers;
            if (parse_object_array(fptr, (void *)&buffers.buffers,
                        &buffers.count, sizeof(struct buffer),
                        (int (*)(FILE *, void *))parse_buffer)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu buffers\n", buffers.count);
        } else if (strcmp(key, "images") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct images images;
            if (parse_object_array(fptr, (void *)&images.images,
                        &images.count, sizeof(struct image),
                        (int (*)(FILE *, void *))parse_image)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu images\n", images.count);
        } else if (strcmp(key, "materials") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct materials materials;
            if (parse_object_array(fptr, (void *)&materials.materials,
                        &materials.count, sizeof(struct material),
                        (int (*)(FILE *, void *))parse_material)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu materials\n", materials.count);
        } else if (strcmp(key, "meshes") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct meshes meshes;
            if (parse_object_array(fptr, (void *)&meshes.meshes,
                        &meshes.count, sizeof(struct mesh),
                        (int (*)(FILE *, void *))parse_meshes)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu meshes\n", meshes.count);
        } else if (strcmp(key, "nodes") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct nodes nodes;
            if (parse_object_array(fptr, (void *)&nodes.nodes,
                        &nodes.count, sizeof(struct node),
                        (int (*)(FILE *, void *))parse_nodes)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu nodes\n", nodes.count);
        } else if (strcmp(key, "samplers") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct samplers samplers;
            if (parse_object_array(fptr, (void *)&samplers.samplers,
                        &samplers.count, sizeof(struct sampler),
                        (int (*)(FILE *, void *))parse_samplers)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu samplers\n", samplers.count);
        } else if (strcmp(key, "scene") == 0) {
            size_t scene;
            PARSE_SIZE_T(fptr, &scene);
            printf("Parsed scene\n");
        } else if (strcmp(key, "scenes") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct scenes scenes;
            if (parse_object_array(fptr, (void *)&scenes.scenes,
                        &scenes.count, sizeof(struct scene),
                        (int (*)(FILE *, void *))parse_scenes)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu scenes\n", scenes.count);
        }

        first = 0;
        free(key);
    }

    return 0;
}

int parse_gltf_file(FILE *fptr) {
    skip_whitespaces(fptr);
    if (fgetc(fptr) != '{')
        return -1;
    return parse_gltf_object(fptr);
}

int gltf_open(const char *filename, struct gjModel *model) {
    (void)model;

    FILE *fptr = fopen(filename, "r");
    if (!fptr) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return -1;
    }

    if (parse_gltf_file(fptr)) {
        fprintf(stderr, "Failed to parse glTF file: %s\n", filename);
        fclose(fptr);
        return -1;
    }

    return 0;
}
