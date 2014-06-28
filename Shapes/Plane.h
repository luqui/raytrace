#ifndef __SHAPES_PLANE_H__
#define __SHAPES_PLANE_H__

#include "Vec.h"
#include "Point.h"

class Plane : public Shape {
    Point origin;
    Vec normal;
public:
    Plane(const Point& origin, const Vec& normal) 
        : origin(origin), normal(normal)
    { }

    void ray_cast(const Ray& cast, RayHit* hit) const {
        // This is a unidirectional plane
        if (cast.direction * normal > 0) {
            hit->did_hit = false;
            return;
        }

        // (cast.origin + t * cast.direction - origin) * normal = 0
        // (cast.origin - origin) * normal + t * cast.direction * normal = 0
        // (cast.origin - origin) * normal = - t * cast.direction * normal
        // -(cast.origin - origin) * normal / (cast.direction * normal) = t
        // (origin - cast.origin) * normal / (cast.direction * normal) = t
        double t = (origin - cast.origin) * normal / (cast.direction * normal);
        if (t > CAST_EPSILON) {
            hit->did_hit = true;
            hit->normal = Ray(cast.origin + t * cast.direction, normal);
            hit->distance2 = (hit->normal.origin - cast.origin).norm2();
        }
        else {
            hit->did_hit = false;
        }
    }
};

#endif
