#include <iostream>
#include <unistd.h>
#include "SDL.h"
#include "SDL_opengl.h"

#include "vec.h"
#include "shape.h"

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    const int WIDTH = 800;
    const int HEIGHT = 600;
    const int BPP = 3;
    SDL_Surface* surface = SDL_SetVideoMode(WIDTH, HEIGHT, 8*BPP, 0);
    if (surface == NULL) {
        std::cerr << "Failed to initialize video mode: " << SDL_GetError() << std::endl;
    }

    SDL_LockSurface(surface);
    for (int x = 0; x < 800; x++) {
        for (int y = 0; y < 600; y++) {
            char* p = (char*)surface->pixels + BPP*(HEIGHT*x+y);
            int h = x*y;
            h *= h;
            h *= h;
            *p++ = (char)(h);
            h >>= 8;
            *p++ = (char)(h);
            h >>= 8;
            *p++ = (char)(h);
        }
    }
    SDL_UnlockSurface(surface);
    SDL_UpdateRect(surface,0,0,0,0);

    sleep(30);
    
    return 0;
}
