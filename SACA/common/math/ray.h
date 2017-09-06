#ifndef RAY_H
#define RAY_H

#include "vec3.h"

#include <float.h>

typedef struct ray ray;

struct ray {
    vec3 pos;
    vec3 dir;
};

// declarations
inline static vec3* ray_nrsttoray(const ray* theray, const ray* other);
inline static float ray_shrtdst(const ray* r1, const ray* r2);
// .declarations

vec3* ray_nrsttoray(const ray* theray, const ray* other)
{
    const vec3* p1 = &theray->pos;
    const vec3* p2 = &other->pos;
    const vec3* d1 = &theray->dir;
    const vec3* d2 = &other->dir;

    vec3* p1p2 = vec3_subtract(p2, p1);

    float p = vec3_dot(d1, d2);
    float q = vec3_dot(d1, p1p2);
    float r = vec3_dot(d2, p1p2);
    float s = vec3_dot(d1, d1);
    float t = vec3_dot(d2, d2);

    if (s == 0) {
        return NULL;
    }

    if (t == 0) {
        return NULL;
    }

    if (p*p == s*t) {
        return NULL;
    }

    float fact = (-p * r + q * t) / (s * t - p * p);

    vec3* d1f = vec3_fmultiply(d1, fact);
    vec3* result = vec3_add(p1, d1f);
    
    free(d1f);
    free(p1p2);

    return result;
}

float ray_shrtdst(const ray* r1, const ray* r2)
{
    const vec3* p1 = &r1->pos;
    const vec3* p2 = &r2->pos;

    const vec3* d1 = &r1->dir;
    const vec3* d2 = &r2->dir;

    vec3* p1p2 = vec3_subtract(p2, p1);

    float p = vec3_dot(d1, d2);
    float q = vec3_dot(d1, p1p2);
    float r = vec3_dot(d2, p1p2);
    float s = vec3_dot(d1, d1);
    float t = vec3_dot(d2, d2);

    if (s == 0) {
        perror("Ray 1 direction vector is zero");
        return FLT_MAX;
    }

    if (t == 0) {
        perror("Ray 2 direction vector is zero");
        return FLT_MAX;
    }

    if (p*p == s*t) {
        perror("Ray 1 and Ray 2 are collinear");
        return FLT_MAX;
    }

    float fact1 = (-p * r + q * t) / (s * t - p * p);
    float fact2 = (p * q - r * s) / (s * t - p * p);

    vec3* d1f = vec3_fmultiply(d1, fact1);
    vec3* d2f = vec3_fmultiply(d2, fact1);

    vec3* np1 = vec3_add(p1, d1f); // nearest point on ray1 to ray2
    vec3* np2 = vec3_add(p2, d2f); // nearest point on ray2 to ray1

    vec3* diff = vec3_subtract(np2, np1);
    float result = vec3_length(diff);

    free(p1p2);
    free(d1f);
    free(d2f);
    free(np1);
    free(np2);
    free(diff);

    return result;
}

#endif
