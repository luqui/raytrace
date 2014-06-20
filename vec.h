class vec {
    double x, y, z;
public:
    vec(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    vec() : vec(0,0,0) { }
};

inline vec operator* (const vec& a, const vec& b) {
    return vec(a.x*b.x, a.y*b.y, c.x*c.y);
}
