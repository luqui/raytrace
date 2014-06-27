#ifndef __FRAME_H__
#define __FRAME_H__

#include "Vec.h"

struct Frame {
    Vec right;
    Vec up;
    Vec forward;

    Frame() : right(Vec(1,0,0)), up(Vec(0,1,0)), forward(Vec(0,0,1)) { }
    Frame(const Vec& right, const Vec& up, const Vec& forward)
        : right(right), up(up), forward(forward)
    { }

    Vec to_global(const Vec& in) {
        return in.x * right + in.y * up + in.z * forward;
    }

    Frame rotate_global(const Vec& axis, double angle) {
        return Frame(right.rotate(axis, angle),
                     up.rotate(axis, angle),
                     forward.rotate(axis, angle));
    }
    
    Frame rotate_local(const Vec& axis, double angle) {
        return rotate_global(to_global(axis), angle);
    }
};

#endif
