#ifndef __RENDER_H__
#define __RENDER_H__

#include "SDL.h"
#include "Shapes/Shape.h"
#include "Vec.h"
#include "Point.h"

const int WIDTH = 640;
const int HEIGHT = 480;
const int BPP = 3;

struct RenderInfo {
    Shape* scene;
    SDL_Surface* surface;
    Vec eye;
    Vec forward;
    Vec up;
    Vec right;
};

inline Color global_ray_cast(RenderInfo* info, int px, int py) {
    double xloc = ((double)px)/WIDTH;
    double yloc = ((double)py)/HEIGHT;
    Vec direction = info->forward - info->right + 2*xloc*info->right
                                  - info->up    + 2*yloc*info->up;
    Ray cast(info->eye, direction.unit());

    double distance = 0;
    for (int casts = 0; casts < 10; ++casts) {
        RayHit hit;
        info->scene->ray_cast(cast, &hit);
        if (hit.did_hit) {
            distance += hit.distance2;
            cast = Ray(hit.ray.origin, Vec::reflect(cast.direction, hit.ray.direction));
        }
        else {
            break;
        }
    }
    double brightness = distance/1000;
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

    // render without forking a thread.
    void render_synch() {
        worker();
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

class SerialRenderer {
    RenderInfo* info;
    RenderWorker worker;
public:
    SerialRenderer(RenderInfo* info) : info(info), worker(info, 0, HEIGHT) { }
    void render() {
        SDL_LockSurface(info->surface);
        worker.render_synch();
        SDL_UnlockSurface(info->surface);
        SDL_Flip(info->surface);
    }
};

#endif
