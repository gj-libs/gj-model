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

float *gj_model_load(const char *filename, int *count);

void   gj_model_free(float *data);

#ifdef __cplusplus
}
#endif

#endif /* GJ_MODEL_H */
