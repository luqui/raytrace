#ifndef __VEC_H__
#define __VEC_H__

#include <cmath>

struct Vec;
Vec operator+ (const Vec&, const Vec&);
Vec operator- (const Vec&, const Vec&);
Vec operator* (double, const Vec&);
Vec operator/ (const Vec&, double);
double operator* (const Vec&, const Vec&);

struct Vec {
    double x, y, z;
    Vec(double x, double y, double z) 
        : x(x), y(y), z(z) 
    { } 
    Vec() : x(0), y(0), z(0) { }

    double norm2() const {
        return x*x + y*y + z*z;
    }

    double norm() const {
        return std::sqrt(norm2());
    }

    Vec unit() const {
        double length = norm();
        if (length == 0) {
            return *this;
        }
        else {
            return *this/length;
        }
    }

    static inline Vec reflect(const Vec& v, const Vec& norm) {
        return v - (2*v*norm)*norm;
    }
};

inline Vec operator+ (const Vec& a, const Vec& b) {
    return Vec(a.x+b.x, a.y+b.y, a.z+b.z);
}

inline Vec operator- (const Vec& a, const Vec& b) {
    return Vec(a.x-b.x, a.y-b.y, a.z-b.z);
}

inline Vec operator- (const Vec& p) {
    return Vec(-p.x, -p.y, -p.z);
}

inline Vec operator* (double a, const Vec& v) {
    return Vec(a*v.x, a*v.y, a*v.z);
}

inline Vec operator/ (const Vec& v, double b) {
    return (1/b)*v;
}

inline double operator* (const Vec& a, const Vec& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline Vec& operator+= (Vec& a, const Vec& b) {
    return a = a + b;
}

#endif
