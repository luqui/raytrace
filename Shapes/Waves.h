#ifndef __SHAPES_WAVES_H__
#define __SHAPES_WAVES_H__

#include <cmath>
#include "Shapes/Shape.h"

class Waves : public Shape {
    Vec vel;     // direction & frequency
    Vec vel_hat; // direction & magnitude
    Shape* child;
public:
    Waves(double amp, const Vec& vel, Shape* child) 
        : vel(vel), child(child)
    {
        vel_hat = amp * vel.unit();
    }
    ~Waves() {
        delete child;
    }

    void ray_cast(const Ray& cast, RayHit* hit) const {
        child->ray_cast(cast, hit);
        if (hit->did_hit) {
            Point p = hit->ray.origin;
            hit->ray.direction = (hit->ray.direction + sin(vel * p.v) * vel_hat).unit();
        }
    }
};

#endif
