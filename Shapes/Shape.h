#ifndef __SHAPES_SHAPE_H__
#define __SHAPES_SHAPE_H__

#include "Vec.h"
#include "Point.h"

class Shape;

struct Ray {
    Ray() { }
    Ray(const Point& o, const Vec& d) : origin(o), direction(d) { }

    Point origin;
    Vec direction;

    Ray reflect(const Vec& normal) const {
        return Ray(origin, direction.reflect(normal));
    }
};

struct RayCast {
    RayCast() : world(NULL), frame_enabled(false) { }
    RayCast(const Ray& ray, Shape* world) : ray(ray), world(world), frame_enabled(false) { }
    Ray ray;
    Shape* world;
    // world properties

    Frame frame;
    bool frame_enabled;

    RayCast reflect(Vec normal) const {
        RayCast ret = RayCast(ray.reflect(normal), world);
        if (frame_enabled) {
            ret.set_frame(frame.reflect(normal));
        }
        return ret;
    }
    
    RayCast rebase(Point base, Vec normal) const {
        RayCast ret = reflect(normal);
        ret.ray.origin = base;
        return ret;
    }

    void set_frame(const Frame& in_frame) {
        frame_enabled = true;
        frame = in_frame;
    }
};

struct RayHit {
    double distance2;

    enum Type { MISS, PORTAL, OPAQUE } type;
    struct Portal {
        RayCast new_cast; 
    } portal;
    
    struct Opaque {
        Ray normal;
    } opaque;
};

class Shape {
public:
    virtual ~Shape() {}

    virtual void ray_cast(const RayCast& cast, RayHit* hit) const = 0;
};

const double CAST_EPSILON = 0.001;

#endif
