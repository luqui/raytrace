#ifndef __COLOR_H__
#define __COLOR_H__

struct Color {
    double red, green, blue;
    Color(double red, double green, double blue) : red(red), green(green), blue(blue) { }
    void to_bytes(unsigned char* r, unsigned char* g, unsigned char* b) {
        *r = (char)(clamp(red,0,1)*255);
        *g = (char)(clamp(green,0,1)*255);
        *b = (char)(clamp(blue,0,1)*255);
    }
};


#endif
