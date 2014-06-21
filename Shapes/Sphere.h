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

    void ray_cast(const Ray& cast, RayHit* hit) const {
        double A = cast.direction.norm2();
        double B = 2*(cast.origin - center) * cast.direction;
        double C = (cast.origin - center).norm2() - radius*radius;

        double disc = B*B - 4*A*C;
        if (disc < 0) {
            hit->did_hit = false;
        }
        else {
            double sqrt_disc = std::sqrt(disc);
            double denom = 1/(2*A);
            double t1 = (-B + sqrt_disc) * denom;
            double t2 = (-B - sqrt_disc) * denom;
            Point hit1 = cast.origin + t1*cast.direction;
            Point hit2 = cast.origin + t2*cast.direction;
            double dist1 = (hit1 - cast.origin).norm2();
            double dist2 = (hit2 - cast.origin).norm2();
            if (t1 > CAST_EPSILON && dist1 <= dist2) {
                hit->did_hit = true;
                hit->distance = sqrt(dist1);
                hit->ray = Ray(hit1, normal_at(hit1));
            }
            else if (t2 > CAST_EPSILON) {
                hit->did_hit = true;
                hit->distance = sqrt(dist2);
                hit->ray = Ray(hit2, normal_at(hit2));
            }
            else {
                hit->did_hit = false;
            }
        }
    }

    Vec normal_at(const Point& p) const {
        return (p - center).unit();
    }
};

#endif