#ifndef __SHAPES_BOUNDINGBOX_H__
#define __SHAPES_BOUNDINGBOX_H__

#include "Shapes/Shape.h"

class BoundingBox : public Shape {
    Point bounds[2];
    Shape* child;
public:
    BoundingBox(const Point& min, const Point& max, Shape* child) : child(child)
    {
        bounds[0] = min;
        bounds[1] = max;
    }
    ~BoundingBox() {
        delete child;
    }

    void ray_cast(const Ray& cast, RayHit* hit) const {
        // http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-box-intersection/

        int signx = cast.direction.x < 0;
        int signy = cast.direction.y < 0;

        double invx = 1/cast.direction.x;
        double invy = 1/cast.direction.y;

        double tmin = (bounds[signx].v.x - cast.origin.v.x) * invx;
        double tmax = (bounds[1-signx].v.x - cast.origin.v.x) * invx;
        double tymin = (bounds[signy].v.y - cast.origin.v.y) * invy;
        double tymax = (bounds[1-signy].v.y - cast.origin.v.y) * invy;
        if ((tmin > tymax) || (tymin > tmax)) { 
            hit->did_hit = false;
            return;
        }
        if (tymin > tmin) { tmin = tymin; }
        if (tymax < tmax) { tmax = tymax; }

        int signz = cast.direction.z < 0;
        double invz = 1/cast.direction.z;
        double tzmin = (bounds[signz].v.z - cast.origin.v.z) * invz;
        double tzmax = (bounds[1-signz].v.z - cast.origin.v.z) * invz;

        if ((tmin > tzmax) || (tzmin > tmax)) { 
            hit->did_hit = false;
            return;
        }
        //if (tzmin > tmin) { tmin = tzmin; }
        //if (tzmax < tmax) { tmax = tzmax; }
        // tmin and tmax now correspond to the two intersections with the AABB,
        // should we need them.

        child->ray_cast(cast, hit);
    }
};

#endif
