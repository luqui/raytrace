#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "SDL.h"
#include "SDL_image.h"

class Image {
    SDL_Surface* surface;

public:
    Image(const char* filename) {
        surface = IMG_Load(filename);
        if (!surface) {
            std::cerr << "Failed to load " << filename << ": " << IMG_GetError() << std::endl;
        }
    }
    ~Image() {
        SDL_FreeSurface(surface);
		
    }

    Color at(double x, double y) const {
#pragma warning(disable:4244)
        int xidx = clamp(int(x * surface->w), 0, surface->w-1);
        int yidx = clamp(int(y * surface->h), 0, surface->h-1);
#pragma warning(default:4244)
        Uint8* pixels = (Uint8*)surface->pixels;
        SDL_PixelFormat* fmt = surface->format;
        Uint32 pixel = *(Uint32*)(&pixels[surface->pitch * yidx + fmt->BytesPerPixel * xidx]);
        Uint8 r = (Uint8)(((pixel & fmt->Rmask) >> fmt->Rshift) << fmt->Rloss);
        Uint8 g = (Uint8)(((pixel & fmt->Gmask) >> fmt->Gshift) << fmt->Gloss);
        Uint8 b = (Uint8)(((pixel & fmt->Bmask) >> fmt->Bshift) << fmt->Bloss);
        double scale = 1/255.0;
        return Color(scale*r, scale*g, scale*b);
    }
};

#endif
