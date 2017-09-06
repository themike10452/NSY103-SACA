#ifndef MATH_H
#define MATH_H

#include <math.h>

#define VERY_SMALL_NUMBER 1.e-8f
#define KM_TO_MILES_RATIO 0.621371f
#define KM_TO_FEET_RATIO 3280.84f

#define toradians(degrees) (degrees * M_PI / 180.0)
#define todegrees(radians) (radians * 180.0 / M_PI)

// declarations
inline static float mod(float num, float dividend);
inline static float clampangle(float degrees);
inline static float kmtomiles(float kilometers);
inline static float kmtofeet(float kilometers);
// .declarations

float mod(float num, float dividend)
{
    if (fabsf( dividend ) < VERY_SMALL_NUMBER)
    {
        return 0.f;
    }

    float quotient = (int) (num / dividend);
    float int_portion = dividend * quotient;

    if (fabsf(int_portion) > fabsf(num))
    {
        int_portion = num;
    }

    return num - int_portion;
}

float clampangle(float degrees)
{
    float angle = mod(degrees, 360.0f);

    if (angle < 0.0f)
        angle += 360.0f;

    return angle;
}

float kmtomiles(float kilometers)
{
    return kilometers * KM_TO_MILES_RATIO;
}

float kmtofeet(float kilometers)
{
    return kilometers * KM_TO_FEET_RATIO;
}

#endif
