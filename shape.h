#ifndef __SHAPE_H__
#define __SHAPE_H__

#include "vec.h"

struct Ray {
    Point p;
    Vec normal;
};

struct RayHit {
    bool did_hit;
    Ray ray;
};

class Shape {
public:
    virtual ~Shape() {}

    virtual void ray_cast(const Ray& cast, RayHit* hit) = 0;
};

class Sphere : public Shape {
    Point center;
    double radius;
public:
};

#endif
