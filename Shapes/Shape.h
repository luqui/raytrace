#ifndef __SHAPES_SHAPE_H__
#define __SHAPES_SHAPE_H__

#include "Vec.h"
#include "Point.h"

struct Ray {
    Ray() { }
    Ray(const Point& o, const Vec& d) : origin(o), direction(d) { }

    Point origin;
    Vec direction;
};

struct RayHit {
    bool did_hit;
    Ray normal;
    double distance2;
};

class Shape {
public:
    virtual ~Shape() {}

    virtual void ray_cast(const Ray& cast, RayHit* hit) const = 0;
};

const double CAST_EPSILON = 0.001;

#endif
