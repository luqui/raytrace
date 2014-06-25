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
    void to_bytes(unsigned char* r, unsigned char* g, unsigned char* b) {
        *r = (unsigned char)(clamp(red,0.0,1.0)*255);
        *g = (unsigned char)(clamp(green,0.0,1.0)*255);
        *b = (unsigned char)(clamp(blue,0.0,1.0)*255);
    }
};


#endif
