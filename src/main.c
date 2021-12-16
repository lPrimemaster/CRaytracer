#include "../inc/common.h"
#include "../inc/caster.h"
#include "../inc/material.h"
#include "../inc/image.h"
#include "../inc/camera.h"
#include "../inc/dispatcher.h"
#include "../inc/display.h"

#include <time.h>

int main(int argc, char* argv[])
{
    init_random();
    
    // Viewport / Image
    const f32 aspect = 16.0f / 9.0f;
    const i32 width = atoi(argv[1]);
    const i32 height = (i32)(width / aspect);
    const i32 spp = atoi(argv[2]);
    const i32 max_depth = atoi(argv[3]);

    // Create hitlist (world)
    hit_list world = new_hit_list();

    material mat_ground = {.type = MTYPE_LAMBERTIAN, .color = {0.5f, 0.5f, 0.5f}};
    sphere ground = { .center = {0, -1000.f, 0}, .radius = 1000.0f, .material = &mat_ground };
    hit_list_add_tail(&world, &ground, HTYPE_SPHERE);


    material mat0 = {.type = MTYPE_DIELECTRIC, .n_refract = 1.5};
    material mat1 = {.type = MTYPE_LAMBERTIAN, .color = {0.4f, 0.2f, 0.1f}};
    material mat2 = {.type = MTYPE_METAL, .color = {0.7f, 0.6f, 0.5f}, .fuzziness = 0.0f};

    sphere s[4] = {
        { .center = {0, 1.0f, 0}, .radius = 1.0f, .material = &mat0 },
        { .center = {-4, 1.0f, 0}, .radius = 1.0f, .material = &mat1 },
        { .center = {4, 1.0f, 0}, .radius = 1.0f, .material = &mat2 },
        { .center = {4, 1.0f, 3.0f}, .radius = 1.0f, .material = &mat1 }
    };

    hit_list_add_tail(&world, &s[0], HTYPE_SPHERE);
    hit_list_add_tail(&world, &s[1], HTYPE_SPHERE);
    hit_list_add_tail(&world, &s[2], HTYPE_SPHERE);
    hit_list_add_tail(&world, &s[3], HTYPE_SPHERE);

    // material mlist[500];
    // sphere   slist[500];
    // i32 i = 0;

    // Add some random spheres
    // for (i32 a = -11; a < 11; a++) 
    // {
    //     for (i32 b = -11; b < 11; b++) 
    //     {
    //         f32 choose_mat = random_f32();
    //         v3_f32 center = {a + 0.9f*random_f32(), 0.2f, b + 0.9f*random_f32()};

    //         v3_f32 ref = {4, 0.2f, 0};

    //         if (v3_f32_len(v3_f32_sub(center, ref)) > 0.9f) 
    //         {
    //             if (choose_mat < 0.8) 
    //             {
    //                 // diffuse
    //                 v3_f32 albedo = v3_f32_random_range(0, 1);
    //                 material m = {.type = MTYPE_LAMBERTIAN, .color = albedo};
    //                 mlist[i] = m;
    //                 sphere s = {.center = center, .radius = 0.2f, .material = &mlist[i]};
    //                 slist[i] = s;
    //                 hit_list_add_tail(&world, &slist[i], HTYPE_SPHERE);
    //             } 
    //             else if (choose_mat < 0.95) 
    //             {
    //                 // metal
    //                 v3_f32 albedo = v3_f32_random_range(0.5, 1);
    //                 f32 fuzz = random_range_f32(0, 0.5);
    //                 material m = {.type = MTYPE_METAL, .color = albedo, .fuzziness = fuzz};
    //                 mlist[i] = m;
    //                 sphere s = {.center = center, .radius = 0.2f, .material = &mlist[i]};
    //                 slist[i] = s;
    //                 hit_list_add_tail(&world, &slist[i], HTYPE_SPHERE);
    //             } 
    //             else 
    //             {
    //                 // glass
    //                 material m = {.type = MTYPE_DIELECTRIC, .n_refract = 1.5};
    //                 mlist[i] = m;
    //                 sphere s = {.center = center, .radius = 0.2f, .material = &mlist[i]};
    //                 slist[i] = s;
    //                 hit_list_add_tail(&world, &slist[i], HTYPE_SPHERE);
    //             }
    //             i++;
    //         }
    //     }
    // }

    // Camera
    v3_f32 cam_pos = {13, 2, 3};
    v3_f32 look_at = {0, 0, 0};
    v3_f32 cam_up  = {0, 1, 0};
    f32    cam_fov = 20.0f;
    f32 dtof = 10.0f;
    f32 apperture = 0.1f;

    cam_info cam = calculate_cam_info(cam_pos, look_at, cam_up, cam_fov, aspect, apperture, dtof);

    // Setup image buffer
    img_buffer* image = new_image_buffer(width, height);

    v3_f32 black = {0.0f, 0.0f, 0.0f};
    image_buffer_set_all(image, black);

    // Render
    ray_dispatcher dispatcher = new_ray_dispatcher(4, 4, width, height, spp, &cam, image);

    // TODO: This is dumb! Refactor -> All inside ray_dispatcher
    ray_job rj1 = {.hl = &world, .depth = max_depth};
    ray_job rj2 = {.hl = &world, .depth = max_depth};
    ray_job rj3 = {.hl = &world, .depth = max_depth};
    ray_job rj4 = {.hl = &world, .depth = max_depth};

    clock_t begin = clock();

    InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x80000400);

    ray_dispatcher_add_job(&dispatcher, &rj1);
    ray_dispatcher_add_job(&dispatcher, &rj2);
    ray_dispatcher_add_job(&dispatcher, &rj3);
    ray_dispatcher_add_job(&dispatcher, &rj4);

    // Render some fancy stuff here using win32 API!
    //Sleep(2000);
    //printf("%x %x %x\n", image->data[10], image->data[20], image->data[30]);
    run_window(image->data, width, height);

    clock_t end = ray_dispatcher_worker_fence(&dispatcher);
    free_ray_dispatcher(&dispatcher);

    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Render time: %lf\n", time_spent);

    write_img_buffer_to_file(image, "output.bmp");

    free_hit_list(&world);

    free_image_buffer(image);

    DeleteCriticalSection(&CriticalSection);
}
