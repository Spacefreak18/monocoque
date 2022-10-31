#ifndef _AC_H
#define _AC_H

#include <stdbool.h>
#include "simapi/acdata.h"

#define AC_PHYSICS_FILE "acpmf_physics"
#define AC_STATIC_FILE "acpmf_static"

typedef struct
{
    bool has_physics;
    bool has_static;
    void* physics_map_addr;
    void* static_map_addr;
    struct SPageFilePhysics ac_physics;
    struct SPageFileStatic ac_static;
}
ACMap;

#endif
