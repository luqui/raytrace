#ifndef __FRAME_H__
#define __FRAME_H__

#include "Vec.h"
#include "Tweaks.h"
#include <math.h>

struct Frame {
    Vec right;
    Vec up;
    Vec forward;

    Frame() { }
    Frame(const Vec& right, const Vec& up, const Vec& forward)
        : right(right), up(up), forward(forward)
    { }

	static Frame from_normal_up(Vec normal, Vec up)
	{
		Frame frame;
		frame.forward = normal;
		frame.up = up;
		frame.right = Vec::cross(up, normal);
		return frame;
	}

    Vec to_global(const Vec& in) const {
        return in.x * right + in.y * up + in.z * forward;
    }

	Vec to_local(const Vec& in) const {
		return Vec(in * right, in * up, in * forward);
	}

	Frame to_global(const Frame& in) const {
		Frame out;
		out.right = to_global(in.right);
		out.up = to_global(in.up);
		out.forward = to_global(in.forward);
		return out;
	}

	Frame to_local(const Frame& in) const {
		Frame out;
		out.right = to_local(in.right);
		out.up = to_local(in.up);
		out.forward = to_local(in.forward);
		return out;
	}

    Frame rotate(const Vec& axis, double angle) const {
        return Frame(right.rotate(axis, angle),
                     up.rotate(axis, angle),
                     forward.rotate(axis, angle));
    }
    
    Frame upright(double dt, const Vec& true_up) const {
        double hand = handedness();
        Vec new_forward = forward.unit();
        Vec new_right = right.flatten(new_forward).unit();
        new_right = new_right.rotate(new_forward, 
                        -std::min(1.0, Tweaks::UPRIGHT_SPEED*hand*dt)*(new_right*true_up));
        Vec new_up = hand * Vec::cross(new_forward, new_right); 
        return Frame(new_right, new_up, new_forward);
    }

    Frame reflect(const Vec& normal) const {
        return Frame(right.reflect(normal), up.reflect(normal), forward.reflect(normal));
    }

    // returns 1 if the frame is right-handed, -1 if it is left-handed
    double handedness() const {
        return sign(Vec::cross(forward, right) * up);
    }
};

#endif
