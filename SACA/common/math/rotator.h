#ifndef ROTATOR_H
#define ROTATOR_H

#include "mat4.h"

typedef struct rotator rotator;

struct rotator {
    float x;
    float y;
    float z;
};

// declarations
inline static vec3* rotator_rotvec3(const rotator* rot, const vec3* v);
// .declarations

vec3* rotator_rotvec3(const rotator* rot, const vec3* v)
{
    mat4* rm = mat4_rotation(rot->x, rot->y, rot->z);
    vec3* result = mat4_transformvec3(rm, v);
    free(rm);
    return result;
}

#endif
