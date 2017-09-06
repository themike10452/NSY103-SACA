#ifndef VEC3_H
#define VEC3_H

#include "../utils/stringutils.h"

#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define VEC3_SERIAL_MAX_LEN 100

typedef struct vec3 vec3;

struct vec3
{
	float x;
	float y;
	float z;
};

// declarations
inline static void  vec3_copy(const vec3* v, vec3* dest);
inline static void  vec3_applyadd(vec3* v, const vec3* other);
inline static void  vec3_applysubtract(vec3* v, const vec3* other);
inline static void  vec3_normalize(vec3* v);
inline static float vec3_length(const vec3* v);
inline static vec3* vec3_getnormal(const vec3* v);
inline static vec3* vec3_add(const vec3* v1, const vec3* v2);
inline static vec3* vec3_subtract(const vec3* v1, const vec3* v2);
inline static vec3* vec3_fmultiply(const vec3* v, float factor);
inline static float vec3_dot(const vec3* v1, const vec3* v2);
inline static float vec3_distance(const vec3* v1, const vec3* v2);
inline static float vec3_xydistance(const vec3* v1, const vec3* v2);
inline static float vec3_zdistance(const vec3* v1, const vec3* v2);
inline static vec3* vec3_deserialize(const char* str);
inline static char* vec3_serialize(const vec3* v);
// .declarations

void vec3_copy(const vec3* v, vec3* dest)
{
	dest->x = v->x;
	dest->y = v->y;
	dest->z = v->z;
}

void vec3_applyadd(vec3* v, const vec3* other)
{
	v->x += other->x;
	v->y += other->y;
	v->z += other->z;
}

void vec3_applysubtract(vec3* v, const vec3* other)
{
	v->x -= other->x;
	v->y -= other->y;
	v->z -= other->z;
}

void vec3_normalize(vec3* v)
{
	const float len = vec3_length(v);
	
	if (len == 0)
		return;
	
	v->x /= len;
	v->y /= len;
	v->z /= len;
}

float vec3_length(const vec3* v)
{
	return sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
}

vec3* vec3_getnormal(const vec3* v)
{
	vec3* n = (vec3*)malloc(sizeof(vec3));
	vec3_copy(v, n);
	vec3_normalize(n);
	return n;
}

vec3* vec3_add(const vec3* v1, const vec3* v2)
{
	vec3* sum = (vec3*)malloc(sizeof(vec3));
	vec3_copy(v1, sum);
	vec3_applyadd(sum, v2);
	return sum;
}

vec3* vec3_subtract(const vec3* v1, const vec3* v2)
{
	vec3* diff = (vec3*)malloc(sizeof(vec3));
	vec3_copy(v1, diff);
	vec3_applysubtract(diff, v2);
	return diff;
}

vec3* vec3_fmultiply(const vec3* v, float factor)
{
	vec3* result = (vec3*)malloc(sizeof(vec3));
    result->x = v->x * factor;
    result->y = v->y * factor;
    result->z = v->z * factor;
    return result;
}

float vec3_dot(const vec3* v1, const vec3* v2)
{
	return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}

float vec3_distance(const vec3* v1, const vec3* v2)
{
	vec3* diff = vec3_subtract(v1, v2);
	float dist = vec3_length(diff);
	free(diff);
	return dist;
}

float vec3_xydistance(const vec3* v1, const vec3* v2)
{
	vec3* c1 = (vec3*)malloc(sizeof(vec3));
	vec3* c2 = (vec3*)malloc(sizeof(vec3));

	vec3_copy(v1, c1);
	vec3_copy(v2, c2);

	c1->z = c2->z = 0;

	float dist = vec3_distance(c1, c2);

	free(c1); free(c2);
	
	return dist;
}

float vec3_zdistance(const vec3* v1, const vec3* v2)
{
	return fabsf(v1->z - v2->z);
}

vec3* vec3_deserialize(const char* str)
{
	vec3* v = NULL;
	
	if (str_startswith(str, "vec3::"))
	{
		int count;
		char* content = str_unwrap(str);
		char** elements = str_split(content, ';', &count);
		v = (vec3*)malloc(sizeof(vec3));
		v->x = atof(elements[0]);
		v->y = atof(elements[1]);
		v->z = atof(elements[2]);

		free(content);
	}

	return v;
}

char* vec3_serialize(const vec3* v)
{
	char* str = malloc(VEC3_SERIAL_MAX_LEN * sizeof(char));
	snprintf(str, VEC3_SERIAL_MAX_LEN, "vec3::<%f;%f;%f>", v->x, v->y, v->z);
	return str;
}

#endif
