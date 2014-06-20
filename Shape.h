#ifndef __SHAPE_H__
#define __SHAPE_H__

#include <cmath>
#include "vec.h"

struct Ray {
    Ray(const Point& o, const Vec& d) : origin(o), direction(d) { }

    Point origin;
    Vec direction;
};

struct RayHit {
    bool did_hit;
    Ray ray;
    double distance;
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
            hit->did_hit = true;

            double sqrt_disc = std::sqrt(disc);
            double denom = 1/(2*A);
            double t1 = (-B + sqrt_disc) * denom;
            double t2 = (-B - sqrt_disc) * denom;
            Point hit1 = cast.origin + t1*cast.direction;
            Point hit2 = cast.origin + t2*cast.direction;
            double dist1 = (hit1 - cast.origin).norm2();
            double dist2 = (hit2 - cast.origin).norm2();
            if (dist1 <= dist2) {
                hit->distance = sqrt(dist1);
                hit->ray = Ray(hit1, normal_at(hit1));
            }
            else {
                hit->distance = sqrt(dist2);
                hit->ray = Ray(hit2, normal_at(hit2));
            }
        }
    }

    Vec normal_at(const Point& p) const {
        return (p - center).unit();
    }
};

#endif
