#include "../inc/common.h"
#include "../inc/caster.h"
#include "../inc/material.h"
#include "../inc/image.h"
#include "../inc/camera.h"
#include "../inc/dispatcher.h"
#include "../inc/display.h"

#include <time.h>

material mat_ground = {
    .type = MTYPE_LAMBERTIAN, 
    .texture = { 
        .color[0] = {1.0f, 1.0f, 1.0f},
        .color[1] = {0.0f, 0.0f, 0.0f},
        .type = TTYPE_CHECKER
    }
};
sphere ground = { .center = {0, -1000.f, 0}, .radius = 1000.0f, .material = &mat_ground };

material mat0 = {.type = MTYPE_DIELECTRIC, .n_refract = 1.5};
material mat1 = {.type = MTYPE_LAMBERTIAN, .texture.color[0] = {0.4f, 0.2f, 0.1f}};
material mat2 = {.type = MTYPE_METAL, .texture.color[0] = {0.7f, 0.6f, 0.5f}, .fuzziness = 0.0f};
material mat3 = {.type = MTYPE_LAMBERTIAN, .texture.type = TTYPE_IMAGE_ALBEDO };
material mat4 = {.type = MTYPE_DIFF_LIGHT, .texture.type = TTYPE_COLOR, .texture.color = {1, 1, 1} };
sphere s[3] = {
    { .center = {0, 1.0f, 0}, .radius = 1.0f, .material = &mat0 },
    { .center = {4, 1.0f, 0}, .radius = 1.0f, .material = &mat2 },
    { .center = {4, 1.0f, 3.0f}, .radius = 1.0f, .material = &mat3 }
};

xy_rect r[1] = {
    { .center = {4, 2, -2}, .dims = {2, 2}, .material = &mat4 }
};

hit_list scene_0()
{
    // Create hitlist (world)
    hit_list world = new_hit_list();

    hit_list_add_tail(&world, &ground, HTYPE_SPHERE);

    mat3.texture.image = read_bitmap_image("earthmap.bmp");

    hit_list_add_tail(&world, &s[0], HTYPE_SPHERE);
    hit_list_add_tail(&world, &s[1], HTYPE_SPHERE);
    hit_list_add_tail(&world, &s[2], HTYPE_SPHERE);

    hit_list_add_tail(&world, &r[0], HTYPE_RECT_XY);

    return world;
}

int main(int argc, char* argv[])
{
    init_random();
    
    // Viewport / Image
    const f32 aspect = 16.0f / 9.0f;
    const i32 width = atoi(argv[1]);
    const i32 height = (i32)(width / aspect);
    const i32 spp = atoi(argv[2]);
    const i32 max_depth = atoi(argv[3]);

    // Camera
    v3_f32 cam_pos = {13, 2, 3};
    v3_f32 look_at = {0, 0, 0};
    v3_f32 cam_up  = {0, 1, 0};
    f32    cam_fov = 20.0f;
    f32 dtof = 10.0f;
    f32 apperture = 0.1f;

    cam_info cam = calculate_cam_info(cam_pos, look_at, cam_up, cam_fov, aspect, apperture, dtof);

    // Setup image buffer
    img_buffer* image = new_image_buffer(width, height, 4);

    v3_f32 black = {0.0f, 0.0f, 0.0f};
    image_buffer_set_all(image, black);

    // Render
    ray_dispatcher dispatcher = new_ray_dispatcher(4, 4, width, height, spp, &cam, image);

    hit_list world = scene_0();

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
    run_window((u32*)image->data, width, height);

    // TODO: Find a way to do performance check properly
    clock_t end = ray_dispatcher_worker_fence(&dispatcher);
    free_ray_dispatcher(&dispatcher);

    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Render time: %lf\n", time_spent);

    write_img_buffer_to_file(image, "output.bmp");

    free_hit_list(&world);

    free_image_buffer(image);

    free_image_buffer(mat3.texture.image);

    DeleteCriticalSection(&CriticalSection);
}
