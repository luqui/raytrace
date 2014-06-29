#ifndef __SHAPES_SPHERE_H__
#define __SHAPES_SPHERE_H__

#include <cmath>
#include "Shapes/Shape.h"
#include "Vec.h"
#include "Point.h"

class Sphere : public Shape {
    Point center;
    double radius;

    World* target_world;
    Point target_center;
    double target_radius;
public:
    Sphere(const Point& center, double radius)
        : center(center), radius(radius), target_world(NULL)
    { }

    void set_target(World* world, Point c, double r) {
        target_world = world;
        target_center = c;
        target_radius = r;
    }

    void compute_reflect(const Point& location, double dist, const RayCast cast, RayHit* hit) const {
        Vec normal = normal_at(location);
        // This check orients the sphere outward, so it's invisible from the inside,
        // and so we don't get trapped inside it.
        if (normal * cast.ray.direction > 0) { 
            hit->type = RayHit::TYPE_MISS;
        }
        else {
            hit->type = RayHit::TYPE_PORTAL;
            hit->distance2 = dist;
            hit->portal.new_cast = cast.rebase(location, normal);
            if (target_world) {
                hit->portal.new_cast.world = target_world;
                hit->portal.new_cast.ray.origin = target_center + target_radius * normal;
            }
        }
    }

    void ray_cast(const RayCast& cast, RayHit* hit) const {
        const Ray& ray = cast.ray;
        double A = ray.direction.norm2();
        double B = 2*(ray.origin - center) * ray.direction;
        double C = (ray.origin - center).norm2() - radius*radius;

        double disc = B*B - 4*A*C;
        if (disc < 0) {
            hit->type = RayHit::TYPE_MISS;
        }
        else {
            double sqrt_disc = std::sqrt(disc);
            double denom = 1/(2*A);
            double t1 = (-B + sqrt_disc) * denom;
            double t2 = (-B - sqrt_disc) * denom;
            Point hit1 = ray.origin + t1*ray.direction;
            Point hit2 = ray.origin + t2*ray.direction;
            double dist1 = (hit1 - ray.origin).norm2();
            double dist2 = (hit2 - ray.origin).norm2();
            if (t1 > CAST_EPSILON && dist1 <= dist2) {
                compute_reflect(hit1, dist1, cast, hit);
            }
            else if (t2 > CAST_EPSILON) {
                compute_reflect(hit2, dist2, cast, hit);
            }
            else {
                hit->type = RayHit::TYPE_MISS;
            }
        }
    }

    Vec normal_at(const Point& p) const {
        return (p - center) / radius;
    }
};

#endif
