#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
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
#include "Shapes/BoundingBox.h"
#include "Render.h"
#include "Tweaks.h"

Shape* make_scene() {
    std::vector<Shape*> shapes;

    // BoundingBoxes are for optimization only.

	/*
    shapes.push_back(new Plane(Point(0, -4, 0), Vec(0, 1, 0)));

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
	*/

	shapes.push_back(new Plane(Point(-5, 0, 0), Vec(1, 0, 0)));
	shapes.push_back(new Plane(Point(5, 0, 0), Vec(-1, 0, 0)));
	shapes.push_back(new Plane(Point(0, 0, -5), Vec(0, 0, 1)));
	shapes.push_back(new Plane(Point(0, 0, 5), Vec(0, 0, -1)));

	shapes.push_back(new BoundingBox(Point(-1, -1, -1), Point(1, 1, 1),
                        new Sphere(Point(0, 0, 0), 1)));

    return new LinearCompound(shapes);
}

void quit() {
    IMG_Quit();
    SDL_Quit();
    exit(0);
}

void screenshot(RenderInfo* in_info) {
    const int width = 1280;
    const int height = 960;
    const int bpp = 3;

    Uint32 rmask, gmask, bmask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
#endif

    RenderInfo* info = new RenderInfo;
    info->scene = in_info->scene;
    info->eye = in_info->eye;
    info->frame = in_info->frame;
    info->width = width;
    info->height = height;
    info->bpp = bpp;
    info->cast_limit = 32;
    info->anti_alias = true;

    OpenGLTextureTarget* target = new OpenGLTextureTarget(info);
    ThreadedRenderer* renderer = new ThreadedRenderer(info, 48);
    target->render(renderer);
    target->prepare();

    glClear(GL_COLOR_BUFFER_BIT);
    target->draw();
    SDL_GL_SwapBuffers();

    PixelBuffer buffer = target->get_buffer();
    SDL_Surface* surface = SDL_CreateRGBSurface(
        0, width, height, 8*bpp,
        rmask, gmask, bmask, 0x00000000);
    SDL_LockSurface(surface);
    memcpy(surface->pixels, buffer.pixels, info->width*info->height*info->bpp);
    SDL_UnlockSurface(surface);

    time_t now = time(NULL);
    std::ostringstream stream;
    stream << "screenshots/screenshot-" << now << ".bmp";
    SDL_SaveBMP(surface, stream.str().c_str());
    SDL_FreeSurface(surface);

    SDL_Delay(2000);
    delete target;
    delete renderer;
    delete info;
}

class Game {
    RenderInfo* info;
    OpenGLTextureTarget* render_target;
    BufRenderer* buf_renderer;

    Uint32 last_ticks;

    int skip_mousemotion;
public:
    Game()
    {
        info = new RenderInfo;
        info->scene = make_scene();
        info->eye = Point(0,0,-5);
        info->frame = Frame(Vec(1,0,0), Vec(0,1,0), Vec(0,0,1));
        info->width = 400;
        info->height = 300;
        info->bpp = 3;
        info->cast_limit = 12;
        info->anti_alias = false;

        render_target = new OpenGLTextureTarget(info);
        buf_renderer = new ThreadedRenderer(info, 2);
        
        last_ticks = SDL_GetTicks();
        skip_mousemotion = 10;
    }

    void step() {
        Uint32 ticks = SDL_GetTicks();
		double dt = std::min((ticks - last_ticks) * 0.001, 0.2);
        last_ticks = ticks;

        Uint8* keys = SDL_GetKeyState(NULL);

        Vec intention;
        if (keys[SDLK_LEFT] || keys[SDLK_a]) { intention -= dt*info->frame.right; }
        if (keys[SDLK_RIGHT] || keys[SDLK_d]) { intention += dt*info->frame.right;; }
        if (keys[SDLK_DOWN] || keys[SDLK_s]) { intention -= dt*info->frame.forward; }
        if (keys[SDLK_UP] || keys[SDLK_w]) { intention += dt*info->frame.forward; }

        intention = intention * Tweaks::MOVEMENT_SPEED;

        int safety = 5;
        while (intention.norm2() > 0 && safety--) {
            RayHit hit;
            RayCast cast(Ray(info->eye, intention.unit()), info->scene);
            cast.set_frame(info->frame);
            info->scene->ray_cast(cast, &hit);
            if (hit.type == RayHit::TYPE_MISS) { break; }
            else if (hit.type == RayHit::TYPE_PORTAL) {
                double distance = std::sqrt(hit.distance2);
                double idistance = intention.norm();
                if (distance <= idistance) {
                    std::cout << "Boing!\n";
                    info->eye = hit.portal.new_cast.ray.origin;
                    info->frame = hit.portal.new_cast.frame;
                    intention = (idistance - distance) * hit.portal.new_cast.ray.direction;
                }
                else {
                    break;
                }
            }
            else {
                abort();
            }
        }
        info->eye += intention;
        
        info->frame = info->frame.upright(dt, Vec(0,1,0));
    }

    void draw() {
        render_target->render(buf_renderer);
        render_target->prepare();
        render_target->draw();
    }

    void event(const SDL_Event& e) {
        switch (e.type) {
            case SDL_QUIT:
                quit();
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit();
                }
                if (e.key.keysym.sym == SDLK_RETURN &&
                    (e.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))) {
                    screenshot(info);
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

    SKYBOX = new Image("sunset.jpg");

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Surface* surface = SDL_SetVideoMode(800, 600, 24, SDL_OPENGL);
    if (surface == NULL) {
        std::cerr << "Failed to initialize video mode: " << SDL_GetError() << std::endl;
    }

    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(0);

    glEnable(GL_TEXTURE_2D);

    Game* game = new Game();

    Uint32 old_ticks = SDL_GetTicks();
    int frames = 0;

    while (true) {
        game->step();

        glClear(GL_COLOR_BUFFER_BIT);
        game->draw();
        SDL_GL_SwapBuffers();

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            game->event(e);
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
