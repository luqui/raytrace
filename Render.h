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
        return 0;
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

inline void render_sdl(SDL_Surface* surface, BufRenderer* buf_renderer) {
    PixelBuffer buffer;
    buffer.pixels = (unsigned char*)surface->pixels;

    SDL_LockSurface(surface);
    buf_renderer->render(buffer);
    SDL_UnlockSurface(surface);
}


class OpenGLTextureTarget {
    GLuint tex_id;
    PixelBuffer buffer;
public:
    OpenGLTextureTarget() {
        glGenTextures(1, &tex_id);
        buffer.pixels = new unsigned char [BPP*WIDTH*HEIGHT];
    }
    ~OpenGLTextureTarget() {
        delete buffer.pixels;
        glDeleteTextures(1, &tex_id);
    }

    void render(BufRenderer* buf_renderer) {
        buf_renderer->render(buffer);
    };

    void prepare() {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, WIDTH, HEIGHT,
                     0, GL_RGB, GL_UNSIGNED_BYTE, buffer.pixels);
    }

    void draw(float alpha = 1) {
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glColor4f(1,1,1,alpha);
        glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex2f(-1, -1);
            glTexCoord2f(1, 0);
            glVertex2f(1, -1);
            glTexCoord2f(1, 1);
            glVertex2f(1, 1);
            glTexCoord2f(0, 1);
            glVertex2f(-1, 1);
        glEnd();
    }
};

class BlendRenderer {
    OpenGLTextureTarget* old_target;
    OpenGLTextureTarget* new_target;
    OpenGLTextureTarget* work_target;

    float time;
    SDL_semaphore* render_sem;
    SDL_semaphore* done_sem;
    
    BufRenderer* buf_renderer;

    static int start_callback(void* data) {
        BlendRenderer* self = (BlendRenderer*)data;
        self->render_thread();
        return 0;
    }

    void render_thread() {
        while(true) {
            SDL_SemWait(render_sem);
            work_target->render(buf_renderer);
            SDL_SemPost(done_sem);
        }
    }
    
public:
    BlendRenderer(BufRenderer* buf_renderer) : buf_renderer(buf_renderer) {
        render_sem = SDL_CreateSemaphore(0);
        done_sem = SDL_CreateSemaphore(1);
        
        old_target = new OpenGLTextureTarget();
        new_target = new OpenGLTextureTarget();
        work_target = new OpenGLTextureTarget();
        time = 0;
    }
    virtual ~BlendRenderer() {
        delete old_target;
        delete new_target;
        delete work_target;
    }
    
    void start() {
        SDL_CreateThread(start_callback, this);
    }

    void step(float dt) {
        time += dt;
        if (time >= 1) {
            SDL_SemWait(done_sem);
            work_target->prepare();
            time -= 1;
            OpenGLTextureTarget* tmp = old_target;
            old_target = new_target;
            new_target = work_target;
            work_target = old_target;

            sim_step();
            
            SDL_SemPost(render_sem);
        }
    }

    virtual void sim_step() { }

    void draw() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        old_target->draw(1);
        new_target->draw(time);            
    }
};

#endif
