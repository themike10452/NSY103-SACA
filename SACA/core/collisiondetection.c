#include "collisiondetection.h"
#include "airplane.h"
#include "../common/math/ray.h"

#define VERTICAL_NOTICE_DISTANCE            0.4572f  // km (equiv to 1500 feet)
#define HORIZONTAL_NOTICE_DISTANCE          15.0f    // km
#define VERTICAL_DISTURBANCE_DISTANCE       0.3048f  // km (equiv to 1000 feet)
#define HORIZONTAL_DISTURBANCE_DISTANCE     9.0f     // km
#define VERTICAL_PANIC_DISTANCE             0.15f    // km
#define HORIZONTAL_PANIC_DISTANCE           1.0f     // km
#define FATAL_DISTANCE                      0.05f    // km (equiv to 50 meters)

void collision_detect_clear(airplane* ap)
{
    ap->flags &= ~AP_MASK_CD;
}

void collision_detect_mark(airplane* target, const airplane* other)
{
    int state = 0;

    ray ray1;
    ray1.dir = target->dir;
    ray1.pos = target->coords;

    ray ray2;
    ray2.dir = other->dir;
    ray2.pos = other->coords;

    float pos_distance = vec3_distance(&ray1.pos, &ray2.pos);
    float pos_xydistance = vec3_xydistance(&ray1.pos, &ray2.pos);
    float pos_zdistance = vec3_zdistance(&ray1.pos, &ray2.pos);

    if (pos_xydistance <= HORIZONTAL_DISTURBANCE_DISTANCE &&
        pos_zdistance <= VERTICAL_DISTURBANCE_DISTANCE)
        state = AP_FLAG_WARN;

    vec3* r1cp = ray_nrsttoray(&ray1, &ray2);
    vec3* r2cp = ray_nrsttoray(&ray2, &ray1);

    int willcross = 0;

    // collision point is null if both rays are collinear
    if (r1cp && r2cp)
    {
        // direction towards nearest point
        vec3* d1 = vec3_subtract(r1cp, &ray1.pos);
        vec3* d2 = vec3_subtract(r2cp, &ray2.pos);

        float dot1 = vec3_dot(d1, &ray1.dir);
        float dot2 = vec3_dot(d2, &ray2.dir);

        // the dot product is positive if the ray direction
        // and collision position vectors point in the same direction
        willcross = dot1 > 0 && dot2 > 0;

        // add the factor of speed
        if (willcross)
        {
            float t1 = vec3_distance(&ray1.pos, r1cp) / fmaxf(target->speed, 0.1f);
            float t2 = vec3_distance(&ray2.pos, r2cp) / fmaxf(other->speed, 0.1f);

            willcross = fabsf(t1 - t2) * 3600 <= 30;
        }
    }

    if (willcross)
    {
        float path_xydistance = vec3_xydistance(r1cp, r2cp);
        float path_zdistance = vec3_zdistance(r1cp, r2cp);

        if (state == AP_FLAG_WARN)
        {
            if (path_xydistance <= HORIZONTAL_PANIC_DISTANCE &&
                path_zdistance <= VERTICAL_PANIC_DISTANCE)
                state = AP_FLAG_DNGR;
        }
        else
        {
            int notice = pos_zdistance <= VERTICAL_NOTICE_DISTANCE &&
                         pos_xydistance <= HORIZONTAL_NOTICE_DISTANCE;

            if (notice)
            {
                if (pos_zdistance <= 0.1524) // same vertical flight level
                {
                    if (path_xydistance < HORIZONTAL_DISTURBANCE_DISTANCE)
                        state = AP_FLAG_WARN;
                }
                else
                {
                    if (path_zdistance < VERTICAL_DISTURBANCE_DISTANCE)
                        state = AP_FLAG_WARN;
                }
            }
        }
    }

    if (pos_distance < FATAL_DISTANCE)
        state = AP_FLAG_DAMG;

    if ((target->flags & AP_MASK_CD) < state)
    {
        target->flags = (target->flags & ~AP_MASK_CD) | state;
    }
}
