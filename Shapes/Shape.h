#ifndef __SHAPES_SHAPE_H__
#define __SHAPES_SHAPE_H__

#include "Vec.h"
#include "Point.h"

struct World;

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
    RayCast(const Ray& ray, World* world) : ray(ray), world(world), frame_enabled(false) { }
    Ray ray;
    World* world;
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

	RayCast rebase(Point base, Point source_origin, Frame source_frame, Point dest_origin, Frame dest_frame) const {
		RayCast ret;
		ret.ray.origin = dest_origin + dest_frame.to_global(source_frame.to_local(base - source_origin));
		ret.ray.direction = dest_frame.to_global(source_frame.to_local(ray.direction));
		ret.world = world;
		if (frame_enabled) {
			ret.set_frame(dest_frame.to_global(source_frame.to_local(frame)));
		}
        return ret;
    }

    void set_frame(const Frame& in_frame) {
        frame_enabled = true;
        frame = in_frame;
    }
};

struct RayHit {
    double distance2;

    enum Type { TYPE_MISS, TYPE_PORTAL, TYPE_OPAQUE } type;

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

class EmptyShape : public Shape {
public:
	void ray_cast(const RayCast& cast, RayHit* hit) const
	{ hit->type = RayHit::TYPE_MISS; }
};

const double CAST_EPSILON = 0.001;

#endif
