#ifndef __RENDER_H__
#define __RENDER_H__

#include "SDL.h"
#include "Shapes/Shape.h"
#include "Vec.h"
#include "Point.h"
#include "Image.h"
#include "Frame.h"

const double PI = 3.14159265358979323846264338327950288;

struct World {
    Image* skybox;
    Shape* scene;
};

struct RenderInfo {
    World* world;
    Point eye;
    Frame frame;
    int width, height, bpp;
    int cast_limit;
    bool anti_alias;
};

struct PixelBuffer {
    unsigned char* pixels;
};

inline Color compute_skybox(const RayCast& cast) {
    double angle_h = 0.5 + (1/(2*PI)) * atan2(cast.ray.direction.x, cast.ray.direction.z);
    double angle_p = 0.5 + (1/PI) * asin(-cast.ray.direction.y);
    return cast.world->skybox->at(angle_h, angle_p);
}

inline Color single_ray_cast(RenderInfo* info, double xloc, double yloc) {
    Vec direction = info->frame.forward - info->frame.right + 2*xloc*info->frame.right
                                        - info->frame.up    + 2*yloc*info->frame.up;
    Ray ray(info->eye, direction.unit());
    RayCast cast(ray, info->world);

    // consider adaptive ray limit based on distance
    for (int casts = 0; casts < info->cast_limit; ++casts) {
        RayHit hit;
        cast.world->scene->ray_cast(cast, &hit);
        switch (hit.type) {
            case RayHit::TYPE_MISS: return compute_skybox(cast);
            case RayHit::TYPE_PORTAL: {
                cast = hit.portal.new_cast;
                break;
            }
            default: abort(); break;
        }
    }
    return compute_skybox(cast);
};

inline Color global_ray_cast(RenderInfo* info, int px, int py) {
    double epsx = 1.0/info->width;
    double epsy = 1.0/info->height;
    double xloc = epsx*px;
    double yloc = epsy*py;
    if (info->anti_alias) {
        return 0.25 * ( single_ray_cast(info, xloc - 0.25*epsx, yloc - 0.25*epsy)
                      + single_ray_cast(info, xloc - 0.25*epsx, yloc + 0.25*epsy)
                      + single_ray_cast(info, xloc + 0.25*epsx, yloc - 0.25*epsy)
                      + single_ray_cast(info, xloc + 0.25*epsx, yloc + 0.25*epsy) );
    }
    else {
        return single_ray_cast(info, xloc, yloc);
    }
}

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
    bool done;

    PixelBuffer buffer;

    void worker() {
        unsigned char* pixels = buffer.pixels;
        for (int x = 0; x < info->width; x++) {
            for (int y = ystart; y < yend; y++) {
                unsigned char* p = pixels + info->bpp*(x+info->width*y);
                Color c = global_ray_cast(info, x, info->height-y);
                c.to_bytes(p, p+1, p+2);
            }
        }
    }

public:
    RenderWorker(RenderInfo* info, int ystart, int yend)
        : info(info), ystart(ystart), yend(yend), done(false)
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
            if (worker->done) { SDL_SemPost(worker->done_mutex); break; }
            worker->worker();
            SDL_SemPost(worker->done_mutex);
        }
        return 0;
    }

    void finish() {
        done = true;
        SDL_SemPost(go_mutex);
        SDL_SemWait(done_mutex);
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
    std::vector<RenderWorker*> workers;
public:
    ~ThreadedRenderer() {
        for (std::vector<RenderWorker*>::iterator i = workers.begin(); i != workers.end(); ++i) {
            (*i)->finish();  // serial blocking termination -- can be done in parallel
            delete *i;
        }
    }
    ThreadedRenderer(RenderInfo* info, int threads) {
        for (int t = 0; t < threads; t++) {
            RenderWorker* worker = new RenderWorker(
                info,
                t*info->height/threads,
                (t+1)*info->height/threads);
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
    RenderWorker worker;
public:
    SerialRenderer(RenderInfo* info) : worker(info, 0, info->height) { }
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
    RenderInfo* info;
    GLuint tex_id;
    PixelBuffer buffer;
public:
    OpenGLTextureTarget(RenderInfo* info) : info(info) {
        glGenTextures(1, &tex_id);
        buffer.pixels = new unsigned char [info->bpp*info->width*info->height];
    }
    ~OpenGLTextureTarget() {
        delete buffer.pixels;
        glDeleteTextures(1, &tex_id);
    }

    void render(BufRenderer* buf_renderer) {
        buf_renderer->render(buffer);
    };

    PixelBuffer get_buffer() const { return buffer; }

    void prepare() {
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, info->width, info->height,
                     0, GL_RGB, GL_UNSIGNED_BYTE, buffer.pixels);
    }

    void draw(double alpha = 1) {
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glColor4d(1,1,1,alpha);
        glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex2f(-1, 1);
            glTexCoord2f(1, 0);
            glVertex2f(1, 1);
            glTexCoord2f(1, 1);
            glVertex2f(1, -1);
            glTexCoord2f(0, 1);
            glVertex2f(-1, -1);
        glEnd();
    }
};

#endif
