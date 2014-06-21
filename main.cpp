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

const int WIDTH = 800;
const int HEIGHT = 600;
const int BPP = 3;


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

struct RenderInfo {
    Shape* scene;
    SDL_Surface* surface;
    Vec eye;
    Vec forward;
    Vec up;
    Vec right;
};


Color global_ray_cast(RenderInfo* info, int px, int py) {
    double xloc = ((double)px)/WIDTH;
    double yloc = ((double)py)/HEIGHT;
    Vec direction = info->forward - info->right + 2*xloc*info->right
                                  - info->up    + 2*yloc*info->up;
    Ray cast(info->eye, direction.unit());

    double brightness = 1;
    for (int casts = 0; casts < 10; ++casts) {
        RayHit hit;
        info->scene->ray_cast(cast, &hit);
        if (hit.did_hit) {
            brightness *= 0.5;
            cast = Ray(hit.ray.origin, Vec::reflect(cast.direction, hit.ray.direction));
        }
        else {
            break;
        }
    }
    return Color(brightness, brightness, brightness);
}


class RenderWorker {
    RenderInfo* info;
    SDL_semaphore* go_mutex;
    SDL_semaphore* done_mutex;
    int ystart;
    int yend;

    void worker() {
        unsigned char* pixels = (unsigned char*)info->surface->pixels;
        for (int x = 0; x < WIDTH; x++) {
            for (int y = ystart; y < yend; y++) {
                unsigned char* p = pixels + BPP*(x+WIDTH*y);
                Color c = global_ray_cast(info, x, HEIGHT-y);
                c.to_bytes(p, p+1, p+2);
            }
        }
    }

public:
    RenderWorker(RenderInfo* info, int ystart, int yend)
        : info(info), ystart(ystart), yend(yend)
    {
        go_mutex = SDL_CreateSemaphore(0);
        done_mutex = SDL_CreateSemaphore(0);
    }

    ~RenderWorker() {
        SDL_DestroySemaphore(go_mutex);
        SDL_DestroySemaphore(done_mutex);
    }

    static int worker_callback(void* data) {
        RenderWorker* worker = (RenderWorker*)data;
        while (true) {
            SDL_SemWait(worker->go_mutex);
            worker->worker();
            SDL_SemPost(worker->done_mutex);
        }
    }

    void start_render() {
        SDL_SemPost(go_mutex);
    }

    void wait() {
        SDL_SemWait(done_mutex);
    }

    void fork() {
        SDL_CreateThread(worker_callback, this);
    }
};

class ThreadedRenderer {
    RenderInfo* info;
    std::vector<RenderWorker*> workers;
public:
    ~ThreadedRenderer() {
        for (std::vector<RenderWorker*>::iterator i = workers.begin(); i != workers.end(); ++i) {
            delete *i;
        }
    }
    ThreadedRenderer(RenderInfo* info, int threads) : info(info) {
        for (int t = 0; t < threads; t++) {
            RenderWorker* worker = new RenderWorker(
                info, 
                t*HEIGHT/threads, 
                (t+1)*HEIGHT/threads);
            worker->fork();
            workers.push_back(worker);
        }
    }
    void render() {
        SDL_LockSurface(info->surface);
        for (std::vector<RenderWorker*>::iterator i = workers.begin(); i != workers.end(); ++i) {
            (*i)->start_render();
        }
        for (std::vector<RenderWorker*>::iterator i = workers.begin(); i != workers.end(); ++i) {
            (*i)->wait();
        }
        SDL_UnlockSurface(info->surface);
        SDL_Flip(info->surface);
    }
};

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
