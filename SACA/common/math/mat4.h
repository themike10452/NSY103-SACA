#ifndef MAT4_H
#define MAT4_H

#include "vec3.h"
#include "math2.h"

#include <math.h>

typedef struct mat4 mat4;

struct mat4
{
    float Data[4][4];
};

// declarations
inline static vec3* mat4_transformvec3(const mat4* m, const vec3* v);
inline static mat4* mat4_rotation(float x, float y, float z);
// .declarations

vec3* mat4_transformvec3(const mat4* m, const vec3* v)
{
    vec3* tv = (vec3*)malloc(sizeof(vec3));
    tv->x = v->x * m->Data[0][0] + v->y * m->Data[1][0] + v->z * m->Data[2][0] + m->Data[3][0];
    tv->y = v->x * m->Data[0][1] + v->y * m->Data[1][1] + v->z * m->Data[2][1] + m->Data[3][1];
    tv->z = v->x * m->Data[0][2] + v->y * m->Data[1][2] + v->z * m->Data[2][2] + m->Data[3][2];
    return tv;
}

mat4* mat4_rotation(float x, float y, float z)
{
    mat4* m = (mat4*)malloc(sizeof(mat4));

    const float cx = cosf(toradians(x));
    const float cy = cosf(toradians(y));
    const float cz = cosf(toradians(z));

    const float sx = sinf(toradians(x));
    const float sy = sinf(toradians(y));
    const float sz = sinf(toradians(z));

    // RxRyRz
    m->Data[0][0]	= cy*cz;
    m->Data[0][1]	= -cy*sz;
    m->Data[0][2]	= sy;
    m->Data[0][3]	= 0.f;

    m->Data[1][0]	= cx*sz+cz*sx*sy;
    m->Data[1][1]	= cx*cz-sx*sy*sz;
    m->Data[1][2]	= -cy*sx;
    m->Data[1][3]	= 0.f;

    m->Data[2][0]	= sx*sz-cx*cz*sy;
    m->Data[2][1]	= cz*sx+cx*sy*sz;
    m->Data[2][2]	= cx*cy;
    m->Data[2][3]	= 0.f;

    m->Data[3][0]	= 0.0f; //x trans
    m->Data[3][1]	= 0.0f; //y trans
    m->Data[3][2]	= 0.0f; //z trans
    m->Data[3][3]	= 1.f;

    return m;
}

#endif
