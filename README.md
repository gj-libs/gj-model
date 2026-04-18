
# gj_model

## Supported file types
- STL - Full support
- OBJ - Partial support

## Output format
- struct Mesh
- Mesh.vertices
    - in the format (x,y,z,nx,ny,nz,u,v)
        - x,y,z    are position coords
        - nx,ny,nz are normals
        - u,v      are texture coords

## Build
```bash
make
```

## Example
```c
#include "gj_model/gj_model.h"

int main() {
    struct Mesh mesh = gj_model_load("model.stl");

    if (!mesh.vertices) return 1;

    gj_model_free(&mesh);

    return 0;
}
```
