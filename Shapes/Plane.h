#ifndef __SHAPES_PLANE_H__
#define __SHAPES_PLANE_H__

#include "Vec.h"
#include "Point.h"

class Plane : public Shape {
    Point origin;
    Vec normal;
	World* target_world;
    Point target_center;

public:
    Plane(const Point& origin, const Vec& normal) 
        : origin(origin), normal(normal)
    {
		target_world = NULL;
	}

	void set_target(World* world, Point c) {
        target_world = world;
        target_center = c;
    }

    void ray_cast(const RayCast& cast, RayHit* hit) const {
        const Ray& ray = cast.ray;
        // This is a unidirectional plane
        if (ray.direction * normal > 0) {
            hit->type = RayHit::TYPE_MISS;
            return;
        }

        // (cast.origin + t * cast.direction - origin) * normal = 0
        // (cast.origin - origin) * normal + t * cast.direction * normal = 0
        // (cast.origin - origin) * normal = - t * cast.direction * normal
        // -(cast.origin - origin) * normal / (cast.direction * normal) = t
        // (origin - cast.origin) * normal / (cast.direction * normal) = t
        double t = (origin - ray.origin) * normal / (ray.direction * normal);
        if (t > CAST_EPSILON) {
            hit->type = RayHit::TYPE_PORTAL;
            Point hit_point = ray.origin + t * ray.direction;
            hit->portal.new_cast = cast.rebase(hit_point, normal);
            hit->distance2 = (hit_point - ray.origin).norm2();
			if (target_world) {
                hit->portal.new_cast.world = target_world;
                hit->portal.new_cast.ray.origin = target_center;
            }
        }
        else {
            hit->type = RayHit::TYPE_MISS;
        }
    }
};

#endif
