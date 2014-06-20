#include <iostream>
#include <vector>
#include <unistd.h>
#include "SDL.h"
#include "SDL_opengl.h"

#include "Vec.h"
#include "Shape.h"

Shape* make_scene() {
    std::vector<Shape*> shapes;
    shapes.push_back(new Sphere(Point(-2, 0, 0), 1));
    shapes.push_back(new Sphere(Point(0, 0, 0), 1));
    shapes.push_back(new Sphere(Point(2, 0, 0), 1));

    return new LinearCompound(shapes);
}


Color global_ray_cast(Shape* scene, const Point& eye,
                      const Vec& forward, const Vec& up, const Vec& right,
                      int resx, int resy, int px, int py) {
    double xloc = ((double)px)/resx;
    double yloc = ((double)py)/resy;
    Vec direction = forward - right + 2*xloc*right
                            - up    + 2*yloc*up;
    Ray cast(eye, direction.unit());

    for (int casts = 0; casts < 10; ++casts) {
        RayHit hit;
        scene->ray_cast(cast, &hit);
        if (hit.did_hit) {
            cast = Ray(hit.ray.origin, Vec::reflect(cast.direction, hit.ray.direction));
        }
        else {
            double brightness = ((double)casts)/4;
            return Color(brightness, brightness, brightness);
        }
    }
    return Color(1,0,1);
}

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

    Shape* scene = make_scene();

    SDL_LockSurface(surface);
    for (int x = 0; x < 800; x++) {
        for (int y = 0; y < 600; y++) {
            unsigned char* p = (unsigned char*)surface->pixels + BPP*(x+WIDTH*y);
            Color c = global_ray_cast(
                scene,
                Point(0, 0, -5),   // center
                Vec(0, 0, 1),       // forward
                Vec(0, 1, 0),       // up
                Vec(1, 0, 0),       // right
                WIDTH, HEIGHT,
                x, HEIGHT-y);

            c.to_bytes(p, p+1, p+2);
        }
    }
    SDL_UnlockSurface(surface);
    SDL_UpdateRect(surface,0,0,0,0);

    sleep(30);

    delete scene;
    
    return 0;
}
