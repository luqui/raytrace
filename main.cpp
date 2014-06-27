#include <iostream>
#include <vector>
#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_image.h"

#include "Vec.h"
#include "Point.h"
#include "Color.h"
#include "Frame.h"
#include "Shapes/Shape.h"
#include "Shapes/Sphere.h"
#include "Shapes/LinearCompound.h"
#include "Shapes/Plane.h"
#include "Shapes/Waves.h"
#include "Shapes/BoundingBox.h"
#include "Render.h"

Shape* make_scene() {
    std::vector<Shape*> shapes;

    // BoundingBoxes are for optimization only.
    
    shapes.push_back(new Waves(0.1, Vec(1,0,1), 
                        new Waves(0.05, Vec(-0.5,0,0.3),
                            new Plane(Point(0, -4, 0), Vec(0, 1, 0)))));

    std::vector<Shape*> leftbox;
    for (double x = -40; x < 0; x += 8) {
        std::vector<Shape*> subshapes;
        subshapes.push_back(new Sphere(Point(x, 0, 0), 1));
        subshapes.push_back(new Sphere(Point(x-2, 0, 0), 1));
        subshapes.push_back(new Sphere(Point(x+2, 0, 0), 1));
        subshapes.push_back(new Sphere(Point(x, 2, 0), 1));
        leftbox.push_back(new BoundingBox(Point(x-3,-1,-1), Point(x+3,3,1),
                            new LinearCompound(subshapes)));
    }
    shapes.push_back(new BoundingBox(Point(-43,-1,-1), Point(3,3,1),
                        new LinearCompound(leftbox)));

    std::vector<Shape*> rightbox;
    for (double x = 0; x <= 40; x += 8) {
        std::vector<Shape*> subshapes;
        subshapes.push_back(new Sphere(Point(x, 0, 0), 1));
        subshapes.push_back(new Sphere(Point(x-2, 0, 0), 1));
        subshapes.push_back(new Sphere(Point(x+2, 0, 0), 1));
        subshapes.push_back(new Sphere(Point(x, 2, 0), 1));
        rightbox.push_back(new BoundingBox(Point(x-3,-1,-1), Point(x+3,3,1),
                            new LinearCompound(subshapes)));
    }
    shapes.push_back(new BoundingBox(Point(-3,-1,-1), Point(43,3,1),
                        new LinearCompound(rightbox)));

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

        Vec intention;
        if (keys[SDLK_LEFT] || keys[SDLK_a]) { intention -= dt*info->frame.right; }
        if (keys[SDLK_RIGHT] || keys[SDLK_d]) { intention += dt*info->frame.right;; }
        if (keys[SDLK_DOWN] || keys[SDLK_s]) { intention -= dt*info->frame.forward; }
        if (keys[SDLK_UP] || keys[SDLK_w]) { intention += dt*info->frame.forward; }

        int safety = 5;
        while (intention.norm2() > 0 && safety--) {
            RayHit hit;
            info->scene->ray_cast(Ray(info->eye, intention.unit()), &hit);
            double distance = (info->eye - hit.ray.origin).norm();
            double idistance = intention.norm();
            if (hit.did_hit && distance <= idistance) {
                info->eye = hit.ray.origin;
                intention = (idistance - distance) * intention.reflect(hit.ray.direction).unit();
                info->frame = info->frame.reflect(hit.ray.direction);
            }
            else {
                break;
            }
        }
        info->eye += intention;
    }
};

void quit() {
    IMG_Quit();
    SDL_Quit();
    exit(0);
}

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (IMG_Init(IMG_INIT_JPG) == 0) {
        std::cerr << "SDL_Image could not be initialized: " << IMG_GetError() << std::endl;
        return 1;
    }

    SKYBOX = new Image("sunset.jpg");

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Surface* surface = SDL_SetVideoMode(800, 600, 8*BPP, SDL_OPENGL);
    if (surface == NULL) {
        std::cerr << "Failed to initialize video mode: " << SDL_GetError() << std::endl;
    }

    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(0);

    glEnable(GL_TEXTURE_2D);

    RenderInfo* info = new RenderInfo;
    info->scene = make_scene();
    info->eye = Point(0,0,-5);

    BufRenderer* buf_renderer = new ThreadedRenderer(info, 2);
    OpenGLTextureTarget gl_target;

    Game* game = new Game(buf_renderer, info);
    game->start();

    Uint32 old_ticks = SDL_GetTicks();
    int frames = 0;

    int skip_mousemotion = 10;

    while (true) {
        game->step(1);

        glClear(GL_COLOR_BUFFER_BIT);
        game->draw();
        SDL_GL_SwapBuffers();

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    quit();
                    break;
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_ESCAPE) {
                        quit();
                    }
                    break;
                case SDL_MOUSEMOTION: {
                    if (skip_mousemotion) { skip_mousemotion--; break; }
                    double xrel = e.motion.xrel / 400.0;
                    double yrel = e.motion.yrel / 300.0;
                    double handedness = info->frame.handedness();
                    double orientation = sign(info->frame.up * Vec(0,1,0));
                    info->frame = info->frame.rotate(Vec(0,1,0), orientation * handedness * xrel)
                                             .rotate(info->frame.right, handedness * yrel);
                    break;
                }
            }
        }
        info->frame = info->frame.upright(Vec(0,1,0));

        frames++;
        if (frames % 30 == 0) {
            Uint32 ticks = SDL_GetTicks();
            std::cout << "FPS: " << frames/(0.001 * (ticks-old_ticks)) << "\n";
            frames = 0;
            old_ticks = ticks;
        }
    }
}
