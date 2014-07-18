#ifndef __SHAPES_PLANE_H__
#define __SHAPES_PLANE_H__

#include "Vec.h"
#include "Point.h"

class Plane : public Shape {
    Point origin;
    Frame frame;
	World* target_world;
	Point target_origin;
    Frame target_frame;

public:
    Plane(const Point& origin, const Frame& frame) 
        : origin(origin), frame(frame)
    {
		target_world = NULL;
	}

	const Vec& normal() const
	{
		return frame.forward;
	}

	void set_target(World* world, const Point& origin, const Frame& frame) {
        target_world = world;
        target_origin = origin;
		target_frame = frame;
    }

    void ray_cast(const RayCast& cast, RayHit* hit) const {
        const Ray& ray = cast.ray;
        // This is a unidirectional plane
        if (ray.direction * normal() > 0) {
            hit->type = RayHit::TYPE_MISS;
            return;
        }

        // (cast.origin + t * cast.direction - origin) * normal = 0
        // (cast.origin - origin) * normal + t * cast.direction * normal = 0
        // (cast.origin - origin) * normal = - t * cast.direction * normal
        // -(cast.origin - origin) * normal / (cast.direction * normal) = t
        // (origin - cast.origin) * normal / (cast.direction * normal) = t
        double t = (origin - ray.origin) * normal() / (ray.direction * normal());
        if (t > CAST_EPSILON) {
            hit->type = RayHit::TYPE_PORTAL;
            Point hit_point = ray.origin + t * ray.direction;
			hit->distance2 = (hit_point - ray.origin).norm2();
			if (!target_world)
			{
				hit->portal.new_cast = cast.rebase(hit_point, normal());
			}
			else {
				hit->portal.new_cast = cast.rebase(hit_point, origin, frame, target_origin, target_frame);
				hit->portal.new_cast.world = target_world;
            }
        }
        else {
            hit->type = RayHit::TYPE_MISS;
        }
    }
};

#endif
