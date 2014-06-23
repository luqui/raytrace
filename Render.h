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
    Vec eye;
    Vec forward;
    Vec up;
    Vec right;
};

struct PixelBuffer {
    unsigned char* pixels;
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
};

class BufRenderer {
public:
    virtual ~BufRenderer() { }
    virtual void render(PixelBuffer buffer) = 0;
};

class RenderWorker {
    RenderInfo* info;
    SDL_semaphore* go_mutex;
    SDL_semaphore* done_mutex;
    int ystart;
    int yend;

    PixelBuffer buffer;

    void worker() {
        unsigned char* pixels = buffer.pixels;
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

    void start_render(PixelBuffer buffer) {
        this->buffer = buffer;
        SDL_SemPost(go_mutex);
    }

    void wait() {
        SDL_SemWait(done_mutex);
    }

    void fork() {
        SDL_CreateThread(worker_callback, this);
    }

    // render without forking a thread.
    void render_synch(PixelBuffer buffer) {
        this->buffer = buffer;
        worker();
    }
};

class ThreadedRenderer : public BufRenderer {
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
    void render(PixelBuffer buffer) {
        for (std::vector<RenderWorker*>::iterator i = workers.begin(); i != workers.end(); ++i) {
            (*i)->start_render(buffer);
        }
        for (std::vector<RenderWorker*>::iterator i = workers.begin(); i != workers.end(); ++i) {
            (*i)->wait();
        }
    }
};

class SerialRenderer : public BufRenderer {
    RenderInfo* info;
    RenderWorker worker;
public:
    SerialRenderer(RenderInfo* info) : info(info), worker(info, 0, HEIGHT) { }
    void render(PixelBuffer buffer) {
        worker.render_synch(buffer);
    }
};


class Renderer {
public:
    virtual ~Renderer() { }
    virtual void render() = 0;
};

class SDLRenderer : public Renderer {
    SDL_Surface* surface;
    BufRenderer* buf_renderer;
public:
    SDLRenderer(SDL_Surface* surface, BufRenderer* buf_renderer)
        : surface(surface), buf_renderer(buf_renderer)
    { }
    void render() {
        PixelBuffer buffer;
        buffer.pixels = (unsigned char*)surface->pixels;

        SDL_LockSurface(surface);
        buf_renderer->render(buffer);
        SDL_UnlockSurface(surface);
    }
};

#endif
