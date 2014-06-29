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

    void ray_cast(const RayCast& cast, RayHit* hit) const {
        // http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-box-intersection/
        const Ray& ray = cast.ray;

        int signx = ray.direction.x < 0;
        int signy = ray.direction.y < 0;

        double invx = 1/ray.direction.x;
        double invy = 1/ray.direction.y;

        double tmin = (bounds[signx].v.x - ray.origin.v.x) * invx;
        double tmax = (bounds[1-signx].v.x - ray.origin.v.x) * invx;
        double tymin = (bounds[signy].v.y - ray.origin.v.y) * invy;
        double tymax = (bounds[1-signy].v.y - ray.origin.v.y) * invy;
        if ((tmin > tymax) || (tymin > tmax)) { 
            hit->type = RayHit::TYPE_MISS;
            return;
        }
        if (tymin > tmin) { tmin = tymin; }
        if (tymax < tmax) { tmax = tymax; }

        int signz = ray.direction.z < 0;
        double invz = 1/ray.direction.z;
        double tzmin = (bounds[signz].v.z - ray.origin.v.z) * invz;
        double tzmax = (bounds[1-signz].v.z - ray.origin.v.z) * invz;

        if ((tmin > tzmax) || (tzmin > tmax)) { 
            hit->type = RayHit::TYPE_MISS;
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
