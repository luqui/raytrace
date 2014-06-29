#ifndef __SHAPES_SPHERE_H__
#define __SHAPES_SPHERE_H__

#include <cmath>
#include "Shapes/Shape.h"
#include "Vec.h"
#include "Point.h"

class Sphere : public Shape {
    Point center;
    double radius;
public:
    Sphere(const Point& center, double radius)
        : center(center), radius(radius)
    { }

    void ray_cast(const RayCast& cast, RayHit* hit) const {
        const Ray& ray = cast.ray;
        double A = ray.direction.norm2();
        double B = 2*(ray.origin - center) * ray.direction;
        double C = (ray.origin - center).norm2() - radius*radius;

        double disc = B*B - 4*A*C;
        if (disc < 0) {
            hit->type = RayHit::MISS;
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
                Vec normal = normal_at(hit1);
                // This check orients the sphere outward, so it's invisible from the inside,
                // and so we don't get trapped inside it.
                if (normal * ray.direction > 0) { 
                    hit->type = RayHit::MISS;
                }
                else {
                    hit->type = RayHit::PORTAL;
                    hit->distance2 = dist1;
                    hit->portal.new_cast = cast.rebase(hit1, normal);
                }
            }
            else if (t2 > CAST_EPSILON) {
                Vec normal = normal_at(hit2);
                if (normal * ray.direction > 0) {
                    hit->type = RayHit::MISS;
                }
                else {
                    hit->type = RayHit::PORTAL;
                    hit->distance2 = dist2;
                    hit->portal.new_cast = cast.rebase(hit2, normal);
                }
            }
            else {
                hit->type = RayHit::MISS;
            }
        }
    }

    Vec normal_at(const Point& p) const {
        return (p - center) / radius;
    }
};

#endif
