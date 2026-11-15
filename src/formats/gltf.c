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

#define PARSE_OBJECT_ARRAY(fptr, array, count, parse_func) \
    do { \
        int capacity = 4; \
        array = malloc(capacity * sizeof(*(array))); \
        count = 0; \
        while (1) { \
            skip_whitespaces(fptr); \
            char c = fgetc(fptr); \
            if (c != '{') { \
                free(array); \
                return -1; \
            } \
            if (count >= capacity) { \
                capacity *= 2; \
                void *new_array = realloc(array, capacity * sizeof(*(array))); \
                if (!new_array) { \
                    free(array); \
                    return -1; \
                } \
                array = new_array; \
            } \
            if (parse_func(fptr, &array[count])) { \
                free(array); \
                return -1; \
            } \
            count++; \
            skip_whitespaces(fptr); \
            c = fgetc(fptr); \
            if (c == ',') { \
                continue; \
            } else if (c == ']') { \
                break; \
            } else { \
                free(array); \
                return -1; \
            } \
        } \
        return 0; \
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
        if (+getc(fptr) != ':')
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
int parse_accessors(FILE *fptr, struct accessors *acs) {
    PARSE_OBJECT_ARRAY(fptr, acs->accessors, acs->count, parse_accessor);
}

int parse_bufferViews(FILE *fptr, struct bufferViews *bvs) {
    PARSE_OBJECT_ARRAY(fptr, bvs->bufferViews, bvs->count, parse_bufferView);
}

int parse_buffers(FILE *fptr, struct buffers *bs) {
    PARSE_OBJECT_ARRAY(fptr, bs->buffers, bs->count, parse_buffer);
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
            if (parse_accessors(fptr, &accessors)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu accessors\n", accessors.count);
        } else if (strcmp(key, "bufferViews") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct bufferViews bufferViews;
            if (parse_bufferViews(fptr, &bufferViews)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu bufferViews\n", bufferViews.count);
        } else if (strcmp(key, "buffers") == 0) {
            c = fgetc(fptr);
            if (c != '[')
                return -1;
            struct buffers buffers;
            if (parse_buffers(fptr, &buffers)) {
                free(key);
                return -1;
            }
            printf("Parsed %zu buffers\n", buffers.count);
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
