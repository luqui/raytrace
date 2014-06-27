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

    Vec to_global(const Vec& in) const {
        return in.x * right + in.y * up + in.z * forward;
    }

    Frame rotate(const Vec& axis, double angle) const {
        return Frame(right.rotate(axis, angle),
                     up.rotate(axis, angle),
                     forward.rotate(axis, angle));
    }
    
    Frame upright(const Vec& true_up) const {
        Vec new_forward = forward.unit();
        Vec new_right = right.flatten(true_up).flatten(new_forward).unit();
        Vec new_up = handedness() * Vec::cross(new_forward, new_right); 
        return Frame(new_right, new_up, new_forward);
    }

    Frame reflect(const Vec& normal) const {
        return Frame(right.reflect(normal), up.reflect(normal), forward.reflect(normal));
    }

    // returns 1 if the frame is right-handed, -1 if it is left-handed
    double handedness() const {
        return Vec::cross(forward, right) * up > 0 ? 1 : -1;
    }
};

#endif
