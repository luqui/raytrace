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

/*
///////////////////////////////////////////////////////////////////
//                  DEBUG TEST WORLD
///////////////////////////////////////////////////////////////////

World* make_compound(Sphere** sphere_out) {
    std::vector<Shape*> shapes;

    // BoundingBoxes are for optimization only.

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
        if (x == 0) {
            *sphere_out = new Sphere(Point(x, 0, 0), 1);
            subshapes.push_back(*sphere_out);
        }
        else {
            subshapes.push_back(new Sphere(Point(x, 0, 0), 1));
        }
        subshapes.push_back(new Sphere(Point(x-2, 0, 0), 1));
        subshapes.push_back(new Sphere(Point(x+2, 0, 0), 1));
        subshapes.push_back(new Sphere(Point(x, 2, 0), 1));
        rightbox.push_back(new BoundingBox(Point(x-3,-1,-1), Point(x+3,3,1),
                            new LinearCompound(subshapes)));
    }
    shapes.push_back(new BoundingBox(Point(-3,-1,-1), Point(43,3,1),
                        new LinearCompound(rightbox)));

    World* world = new World;
    world->scene = new LinearCompound(shapes);
    return world;
}

World* make_world() {
    Sphere* red_sphere;
    Sphere* blue_sphere;
    World* red_world = make_compound(&red_sphere);
    World* blue_world = make_compound(&blue_sphere);

    red_world->skybox = new Image("sunset.jpg");
    blue_world->skybox = new Image("bluesky.jpg");
    red_sphere->set_target(blue_world, Point(0, 0, 0), 1);
    blue_sphere->set_target(red_world, Point(0, 0, 0), 1);

    return red_world;
}
*/

///////////////////////////////////////////////////////////////////
//                  MAX LEVEL
///////////////////////////////////////////////////////////////////

World * make_world_a() {

	World* star_world = new World;
	star_world->scene = new Plane(Point(0, -10, 0), Vec(0, 1, 0));
	star_world->skybox = new Image("starfield.jpg");

	World* worlds[3][3][3];

	// TODO: how do I do this as part of the declaration?
	for (int x = 0; x < 3; ++x)
	{
		for (int y = 0; y < 3; ++y)
		{
			for (int z = 0; z < 3; ++z)
			{
				worlds[x][y][z] = new World;
			}
		}
	}

	int grid_spacing = 5;
	int grid_size = 3;

	for (int x = 0; x < grid_size; ++x)
	{
		for (int y = 0; y < grid_size; ++y)
		{
			for (int z = 0; z < grid_size; ++z)
			{
				std::vector<Shape*> shapes;
				Plane* floor = new Plane(Point(0, -grid_spacing, 0), Vec(0, 1, 0));
				floor->set_target(worlds[x][y > 0 ? y - 1 : grid_size - 1][z], Point(0, -grid_spacing, 0));
				shapes.push_back(floor);

				Plane* ceiling = new Plane(Point(0, grid_spacing, 0), Vec(0, -1, 0));
				ceiling->set_target(worlds[x][y < grid_size - 1 ? y + 1 : 0][z], Point(0, grid_spacing, 0));
				shapes.push_back(ceiling);

				//Plane* left = new Plane(Point(-grid_spacing, 0, 0), Vec(1, 0, 0));
				//left->set_target(worlds[x > 0 ? x - 1 : grid_size - 1][y][z], Point(-grid_spacing, 0, 0));
				//shapes.push_back(left);

				//Plane* right = new Plane(Point(grid_spacing, 0, 0), Vec(-1, 0, 0));
				//right->set_target(worlds[x < grid_size - 1 ? x + 1 : 0][y][z], Point(grid_spacing, 0, 0));
				//shapes.push_back(right);

				//Plane* back = new Plane(Point(0, 0, -grid_spacing), Vec(0, 0, 1));
				//back->set_target(worlds[x][y][z > 0 ? z - 1 : grid_size - 1], Point(0, 0, -grid_spacing));
				//shapes.push_back(back);

				//Plane* front = new Plane(Point(0, 0, grid_spacing), Vec(0, 0, -1));
				//front->set_target(worlds[x][y][z < grid_size - 1 ? z + 1 : 0], Point(0, 0, grid_spacing));
				//shapes.push_back(front);

				Sphere* sphere = new Sphere(Point(0, 0, 0), 1);				
				if (x == 2 && y == 2 && z == 2)
				{
					// Periodic star sphere.
					sphere->set_target(star_world, Point(0, 0, 0), 1);
				}
				shapes.push_back(new BoundingBox(Point(-1,-1,-1), Point(1,1,1), sphere));

				worlds[x][y][z]->scene = new LinearCompound(shapes);
				
				Image* skybox;
				if (y == 0)
				{
					skybox = new Image("bluesky.jpg");
				}
				else if (y == 1)
				{
					skybox = new Image("sunset.jpg");
				}
				else
				{
					skybox = new Image("forest.jpg");
				}
				worlds[x][y][z]->skybox = skybox;
			}
		}
	}

	return worlds[1][1][1];
}

World* make_world() {
    //blue_world->skybox = new Image("bluesky.jpg");
    //red_sphere->set_target(blue_world, Point(0, 0, 0), 1);
    //blue_sphere->set_target(red_world, Point(0, 0, 0), 1);

    return make_world_a();
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
    info->world = in_info->world;
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
        info->world = make_world();
        info->eye = Point(0,0,-3);
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
            RayCast cast(Ray(info->eye, intention.unit()), info->world);
            cast.set_frame(info->frame);
            info->world->scene->ray_cast(cast, &hit);
            if (hit.type == RayHit::TYPE_MISS) { break; }
            else if (hit.type == RayHit::TYPE_PORTAL) {
                double distance = std::sqrt(hit.distance2);
                double idistance = intention.norm();
                if (distance <= idistance) {
                    std::cout << "Boing!\n";
                    info->eye = hit.portal.new_cast.ray.origin;
                    info->frame = hit.portal.new_cast.frame;
                    info->world = hit.portal.new_cast.world;
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
