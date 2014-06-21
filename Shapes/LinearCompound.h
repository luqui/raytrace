#ifndef __SHAPES_LINEARCOMPOUND_H__
#define __SHAPES_LINEARCOMPOUND_H__

#include <vector>
#include "Shapes/Shape.h"
#include "Vec.h"
#include "Point.h"

class LinearCompound : public Shape {
    const std::vector<Shape*> shapes;
public:
    ~LinearCompound() {
        for (std::vector<Shape*>::const_iterator i = shapes.begin(); i != shapes.end(); ++i) {
            delete *i;
        }
    }
    LinearCompound(const std::vector<Shape*>& shapes) : shapes(shapes) { }

    void ray_cast(const Ray& cast, RayHit* hit) const {
        RayHit try_ray;
        RayHit best_ray;
        best_ray.did_hit = false;
        best_ray.distance = INFINITY;

        for (std::vector<Shape*>::const_iterator i = shapes.begin(); i != shapes.end(); ++i) {
            (*i)->ray_cast(cast, &try_ray);
            if (try_ray.did_hit && try_ray.distance < best_ray.distance) {
                best_ray = try_ray;
            }
        }
        *hit = best_ray;
    }
};

#endif
