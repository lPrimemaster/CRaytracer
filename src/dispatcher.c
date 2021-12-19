#include "../inc/dispatcher.h"
#include "../inc/profiler.h"

static ray_job jobs[MAX_THREADS];

// TODO: Add a gradual image add mode
// The idea is one could see an image form from 1 to spp samples

static void job_func(ray_dispatcher* rd, u8 job_id)
{
    f32 scale = 1.0f / rd->spp;
    for(i32 j = job_id * rd->block_size; j < (job_id + 1) * rd->block_size; j++)
    {
        for(i32 i = 0; i < rd->block_width; i++)
        {
            LARGE_INTEGER perf_counter = start_performance_us();
            i32 shot_rays = 0;
            v3_f32 pixel_color = {0, 0, 0};
            for(i32 s = 0; s < rd->spp; s++)
            {
                f32 u = (f32)(i + random_f32()) / (rd->block_width-1);
                f32 v = (f32)(j + random_f32()) / (rd->block_height-1);

                ray r = ray_from_cam_info(rd->cam, u, v);
                
                pixel_color = v3_f32_add(pixel_color, ray_color(&r, rd->list, rd->max_depth, &shot_rays));
            }

            pixel_color = v3_f32_scalar_mult(pixel_color, scale);
            pixel_color = v3_f32_comp_sqrt(pixel_color); // For a gamma of 2.0

            EnterCriticalSection(&CriticalSection);
            // image_buffer_add_pixel(rd->image, i, rd->block_height - j - 1, pixel_color);
            image_buffer_set_pixel(rd->image, i, rd->block_height - j - 1, pixel_color);
            LeaveCriticalSection(&CriticalSection);
            u64 spp_us = query_performance_us(perf_counter);
            _InterlockedExchange64(&rd->atomic_spp_counter[job_id], spp_us);
            _InterlockedExchangeAdd64(&rd->atomic_spp_counter_total[job_id], spp_us);
            _InterlockedExchange(&rd->atomic_spp_rays[job_id], shot_rays);
            _InterlockedExchangeAdd(&rd->atomic_spp_rays_total[job_id], shot_rays);

            if(!_InterlockedCompareExchange16(&rd->atomic_window_running, 0, -1))
            {
                goto stop;
            }
        }
    }

stop:
    printf("Thread #%d: Job finished!\n", job_id);
    _InterlockedExchange64(&rd->atomic_spp_counter[job_id], 1); // Avoid div by zero on display.c
    _InterlockedExchange(&rd->atomic_spp_rays[job_id], 0);
}

static VOID CALLBACK runFunc(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
   ray_job job = *((ray_job*)Context);
   job_func(job.disp, job.id);
}

ray_dispatcher new_ray_dispatcher(u8 num_threads, u8 blocks, i32 width, i32 height, i32 spp, i32 max_depth, cam_info* cam, img_buffer* img, hit_list* list)
{
    if(num_threads > MAX_THREADS) num_threads = MAX_THREADS;
    ray_dispatcher d = {
        .thread_count = num_threads, 
        .dispatch_mode = DMODE_PARALLELW, 
        .last_id = 0, 
        .max_depth = max_depth, 
        .list = list,
        .atomic_window_running = 1
    };

    InitializeThreadpoolEnvironment(&d.cbe);

    d.pool = CreateThreadpool(NULL);
    InitializeThreadpoolEnvironment(&d.cbe);
    d.cug = CreateThreadpoolCleanupGroup();

    SetThreadpoolCallbackPool(&d.cbe, d.pool);
    SetThreadpoolCallbackCleanupGroup(&d.cbe, d.cug, NULL);

    SetThreadpoolThreadMaximum(d.pool, num_threads);
    SetThreadpoolThreadMinimum(d.pool, num_threads);

    d.workers = malloc(sizeof(PTP_WORK) * num_threads);
    d.block_size = height / (i32)blocks;
    d.block_width = width;
    d.block_height = height;
    d.spp = spp;
    d.cam = cam;
    d.image = img;

    return d;
}

void free_ray_dispatcher(ray_dispatcher* rd)
{
    CloseThreadpoolCleanupGroup(rd->cug);
    CloseThreadpool(rd->pool);
    free(rd->workers);
}

void ray_dispatcher_run_jobs(ray_dispatcher* rd)
{
    for(i32 i = 0; i < rd->thread_count; i++)
    {
        rd->workers[rd->last_id] = CreateThreadpoolWork(runFunc, (void*)&jobs[rd->last_id], &rd->cbe);
        jobs[rd->last_id].id = rd->last_id;
        jobs[rd->last_id].disp = rd;
        SubmitThreadpoolWork(rd->workers[rd->last_id]);
        if(rd->last_id <= rd->thread_count) rd->last_id++;
    }
}

clock_t ray_dispatcher_worker_fence(ray_dispatcher* rd)
{
    for(u8 i = 0; i < rd->thread_count; i++)
    {
        WaitForThreadpoolWorkCallbacks(rd->workers[i], FALSE);
        CloseThreadpoolWork(rd->workers[i]);
    }
    return clock();
}
    