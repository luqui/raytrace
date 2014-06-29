#ifndef __POINT_H__
#define __POINT_H__

#include "Vec.h"

struct Point {
    Vec v;
    Point() { }
    explicit Point(Vec v) : v(v) { }
    Point(double x, double y, double z) : v(x,y,z) { }
};

inline Point operator+ (const Point& p, const Vec& v) {
    return Point(p.v + v);
}

inline Point operator- (const Point& p, const Vec& v) {
    return Point(p.v - v);
}

inline Vec operator- (const Point& p, const Point& q) {
    return p.v - q.v;
}

inline Point& operator+= (Point& a, const Vec& b) {
    a.v += b;
    return a;
}

inline Point& operator-= (Point& a, const Vec& b) {
    a.v -= b;
    return a;
}

#endif
