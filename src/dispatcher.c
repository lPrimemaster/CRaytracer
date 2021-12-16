#include "../inc/dispatcher.h"

static void job_func(ray_dispatcher* rd, hit_list* hl, i32 d, u8 job_id)
{
    for(i32 j = job_id * rd->block_size; j < (job_id + 1) * rd->block_size; j++)
    {
        // printf("Thread #%d: Line %d\n", job_id, j);
        for(i32 i = 0; i < rd->block_width; i++)
        {
            v3_f32 pixel_color = {0, 0, 0};
            for(i32 s = 0; s < rd->spp; s++)
            {
                f32 u = (f32)(i + random_f32()) / (rd->block_width-1);
                f32 v = (f32)(j + random_f32()) / (rd->block_height-1);

                ray r = ray_from_cam_info(rd->cam, u, v);
                
                pixel_color = v3_f32_add(pixel_color, ray_color(&r, hl, d));
            }

            pixel_color = v3_f32_scalar_mult(pixel_color, 1.0f / rd->spp);
            pixel_color = v3_f32_comp_sqrt(pixel_color); // For a gamma of 2.0

            EnterCriticalSection(&CriticalSection);
            image_buffer_set_pixel(rd->image, i, rd->block_height - j - 1, pixel_color);
            LeaveCriticalSection(&CriticalSection);
        }
    }

    printf("Thread #%d: Job finished!\n", job_id);
}

static VOID CALLBACK runFunc(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
   ray_job job = *((ray_job*)Context);
   job_func(job.disp, job.hl, job.depth, job.id);
}

ray_dispatcher new_ray_dispatcher(u8 num_threads, u8 blocks, i32 width, i32 height, i32 spp, cam_info* cam, img_buffer* img)
{
    if(num_threads > MAX_THREADS) num_threads = MAX_THREADS;
    ray_dispatcher d = {.thread_count = num_threads, .dispatch_mode = DMODE_PARALLELW, .last_id = 0};

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

void ray_dispatcher_add_job(ray_dispatcher* rd, ray_job* rj)
{
    rd->workers[rd->last_id] = CreateThreadpoolWork(runFunc, (void*)rj, &rd->cbe);
    rj->id = rd->last_id;
    rj->disp = rd;
    SubmitThreadpoolWork(rd->workers[rd->last_id]);
    if(rd->last_id <= rd->thread_count) rd->last_id++;
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
    