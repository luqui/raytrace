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

    Frame upright(const Vec& true_up) {
        Vec new_right = (right - (right*true_up)*true_up).unit();
        if (new_right.norm2() < 0.001) { return *this; } // degenerate
        Vec new_forward = forward.unit();
        Vec new_up = Vec::cross(new_forward, new_right);
        return Frame(new_right, new_up, new_forward);
    }
};

#endif
