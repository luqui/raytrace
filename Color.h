#ifndef __COLOR_H__
#define __COLOR_H__

template<class T>
inline double clamp(T x, T min, T max) {
    if (x < min) { return min; }
    if (x > max) { return max; }
    return x;
}

struct Color {
    double red, green, blue;
    Color(double red, double green, double blue) : red(red), green(green), blue(blue) { }
    void to_bytes(unsigned char* r, unsigned char* g, unsigned char* b) const {
        *r = (unsigned char)(clamp(red,0.0,1.0)*255);
        *g = (unsigned char)(clamp(green,0.0,1.0)*255);
        *b = (unsigned char)(clamp(blue,0.0,1.0)*255);
    }
};

inline Color operator* (double s, const Color& color) {
    return Color(s*color.red, s*color.green, s*color.blue);
}

inline Color operator+ (const Color& a, const Color& b) {
    return Color(a.red+b.red, a.green+b.green, a.blue+b.blue);
}


#endif
