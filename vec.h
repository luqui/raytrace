#ifndef __VEC_H__
#define __VEC_H__

#include <cmath>

struct vec;
vec operator+ (const vec&, const vec&);
vec operator- (const vec&, const vec&);
vec operator* (double, const vec&);
vec operator/ (const vec&, double);
double operator* (const vec&, const vec&);

struct vec {
    const double x, y, z;
    vec(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    vec() : x(0), y(0), z(0) { }

    double norm2() {
        return x*x + y*y + z*z;
    }

    double norm() {
        return std::sqrt(norm2());
    }

    vec unit() {
        double length = norm();
        if (length == 0) {
            return *this;
        }
        else {
            return *this/length;
        }
    }

    static inline vec reflect(const vec& v, const vec& norm) {
        return v - (2*v*norm)*norm;
    }
};

inline vec operator+ (const vec& a, const vec& b) {
    return vec(a.x+b.x, a.y+b.y, a.z+b.z);
}

inline vec operator- (const vec& a, const vec& b) {
    return vec(a.x-b.x, a.y-b.y, a.z-b.z);
}

inline vec operator* (double a, const vec& v) {
    return vec(a*v.x, a*v.y, a*v.z);
}

inline vec operator/ (const vec& v, double b) {
    return (1/b)*v;
}

inline double operator* (const vec& a, const vec& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}


struct point {
    const vec v;
    point(vec v) : v(v) { }
    point(double x, double y, double z) : v(x,y,z) { }
};

inline point operator+ (const point& p, const vec& v) {
    return point(p.v + v);
}

inline point operator- (const point& p, const vec& v) {
    return point(p.v - v);
}

#endif
