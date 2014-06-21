#include <iostream>
#include <vector>
#include "SDL.h"
#include "SDL_opengl.h"

#include "Vec.h"
#include "Point.h"
#include "Color.h"
#include "Shapes/Shape.h"
#include "Shapes/Sphere.h"
#include "Shapes/LinearCompound.h"
#include "Render.h"

Shape* make_scene() {
    std::vector<Shape*> shapes;
    shapes.push_back(new Sphere(Point(-2, 0, 0), 1));
    shapes.push_back(new Sphere(Point(0, 0, 0), 1));
    shapes.push_back(new Sphere(Point(2, 0, 0), 1));
    shapes.push_back(new Sphere(Point(0, 2, 0), 1));
    shapes.push_back(new Sphere(Point(0, -2, 0), 1));
    shapes.push_back(new Sphere(Point(0, 0, 2), 1));

    return new LinearCompound(shapes);
}

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Surface* surface = SDL_SetVideoMode(WIDTH, HEIGHT, 8*BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (surface == NULL) {
        std::cerr << "Failed to initialize video mode: " << SDL_GetError() << std::endl;
    }


    RenderInfo info;
    info.scene = make_scene();
    info.surface = surface;
    info.eye = Vec(0,0,-5);
    info.forward = Vec(0,0,1);
    info.up = Vec(0,1,0);
    info.right = Vec(1,0,0);

    ThreadedRenderer renderer(&info, 16);

    double t = 0;
    while (true) {
        t += 0.05;
        info.eye = Vec(2*sin(t),2*cos(t),-5);
        renderer.render();
    }

    SDL_Quit();
    return 0;
}
