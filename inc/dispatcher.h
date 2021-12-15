#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_
#include "common.h"
#include "caster.h"
#include "material.h"
#include "image.h"
#include "camera.h"
#include <windows.h> // Just because I'm on Windows...

#define MAX_THREADS 4

#define DMODE_PARALLELW 0

typedef struct
{
    PTP_POOL pool;
    TP_CALLBACK_ENVIRON cbe;
    PTP_CLEANUP_GROUP cug;
    u8 thread_count;
    u8 dispatch_mode;
    PTP_WORK* workers;
    u8 last_id;
    i32 block_size;
    i32 block_width;
    i32 block_height;
    i32 spp;
    cam_info* cam;
    img_buffer* image;
} ray_dispatcher;

typedef struct
{
    hit_list* hl;
    i32 depth;
    u8 id;
    ray_dispatcher* disp;
} ray_job;

ray_dispatcher new_ray_dispatcher(u8 num_threads, i32 width, i32 height, i32 spp, cam_info* cam, img_buffer* img);
void free_ray_dispatcher(ray_dispatcher* rd);

void ray_dispatcher_add_job(ray_dispatcher* rd, ray_job* rj);
void ray_dispatcher_worker_fence(ray_dispatcher* rd);


#endif