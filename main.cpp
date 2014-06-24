#include <iostream>
#include <vector>
#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_image.h"

#include "Vec.h"
#include "Point.h"
#include "Color.h"
#include "Shapes/Shape.h"
#include "Shapes/Sphere.h"
#include "Shapes/LinearCompound.h"
#include "Shapes/Plane.h"
#include "Render.h"

Shape* make_scene() {
    std::vector<Shape*> shapes;
    shapes.push_back(new Sphere(Point(-2, 0, 0), 1));
    shapes.push_back(new Sphere(Point(0, 0, 0), 1));
    shapes.push_back(new Sphere(Point(2, 0, 0), 1));
    shapes.push_back(new Sphere(Point(0, 2, 0), 1));
    shapes.push_back(new Sphere(Point(0, -2, 0), 1));
    shapes.push_back(new Sphere(Point(0, 0, 2), 1));
    shapes.push_back(new Plane(Point(0, -4, 0), Vec(0, 1, 0)));

    return new LinearCompound(shapes);
}

class Game : public BlendRenderer {
    RenderInfo* info;
public:
    Game(BufRenderer* buf_renderer, RenderInfo* info) 
        : BlendRenderer(buf_renderer), info(info)
    { }

    void sim_step() {
        double dt = 0.2;
        
        Uint8* keys = SDL_GetKeyState(NULL);
        if (keys[SDLK_LEFT]) { info->eye.x -= dt; }
        if (keys[SDLK_RIGHT]) { info->eye.x += dt; }
        if (keys[SDLK_DOWN]) { info->eye.z -= dt; }
        if (keys[SDLK_UP]) { info->eye.z += dt; }
    }
};

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (IMG_Init(IMG_INIT_JPG) == 0) {
        std::cerr << "SDL_Image could not be initialized: " << IMG_GetError() << std::endl;
        return 1;
    }

    SKYBOX = IMG_Load("sunset.jpg");

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Surface* surface = SDL_SetVideoMode(800, 600, 8*BPP, SDL_OPENGL);
    if (surface == NULL) {
        std::cerr << "Failed to initialize video mode: " << SDL_GetError() << std::endl;
    }

    glEnable(GL_TEXTURE_2D);

    RenderInfo* info = new RenderInfo;
    info->scene = make_scene();
    info->eye = Vec(0,0,-5);
    info->forward = Vec(0,0,1);
    info->up = Vec(0,1,0);
    info->right = Vec(1,0,0);

    BufRenderer* buf_renderer = new ThreadedRenderer(info, 2);
    OpenGLTextureTarget gl_target;

    Game* game = new Game(buf_renderer, info);
    game->start();

    Uint32 old_ticks = SDL_GetTicks();
    int frames = 0;

    while (true) {
        game->step(1);
        
        glClear(GL_COLOR_BUFFER_BIT);
        game->draw();
        SDL_GL_SwapBuffers();

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    IMG_Quit();
                    SDL_Quit();
                    exit(0);
                    break;
            }
        }

        frames++;
        if (frames % 30 == 0) {
            Uint32 ticks = SDL_GetTicks();
            std::cout << "FPS: " << frames/(0.001 * (ticks-old_ticks)) << "\n";
            frames = 0;
            old_ticks = ticks;
        }
    }
}
