#include "../inc/common.h"
#include "../inc/caster.h"
#include "../inc/material.h"
#include "../inc/image.h"
#include "../inc/camera.h"

v3_f32 ray_color(ray* r, hit_list* list, i32 depth)
{
    static const v3_f32 sc = {0, 0, -1};
    static const v3_f32 white = { 1.0f, 1.0f, 1.0f };
    static const v3_f32 black = { 0.0f, 0.0f, 0.0f };
    static const v3_f32 blueish = { 0.5f, 0.7f, 1.0f };

    hit_record rec;

    if(depth <= 0)
        return black;

    if(hit_list_hit_all(list, r, 0.001f, 0xFFFFFF, &rec))
    {
        ray scattered;

        switch (rec.m->type)
        {
        case MTYPE_LAMBERTIAN:
            if(ray_scatter_lambertian(r, &rec, rec.m->color, &scattered))
            {
                return v3_f32_mult(rec.m->color, ray_color(&scattered, list, depth - 1));
            }
            break;
        case MTYPE_METAL:
            if(ray_scatter_metal(r, &rec, rec.m->color, &scattered))
            {
                return v3_f32_mult(rec.m->color, ray_color(&scattered, list, depth - 1));
            }
            break;
        case MTYPE_DIELECTRIC:
            if(ray_scatter_dielectric(r, &rec, rec.m->color, &scattered))
            {
                return v3_f32_mult(white, ray_color(&scattered, list, depth - 1));
            }
            break;
        }

        return black;
    }
    
    v3_f32 u_dir = v3_f32_to_unit(r->d);
    f32 t = 0.5f * (u_dir.y + 1.0f);


    return v3_f32_add(v3_f32_scalar_mult(white, (1.0f - t)), v3_f32_scalar_mult(blueish, t));
}

int main(int argc, char* arg[])
{
    init_random();
    
    // Viewport / Image
    const f32 aspect = 16.0f / 9.0f;
    const i32 width = 1920;
    const i32 height = (i32)(width / aspect);
    const i32 spp = 64;
    const i32 max_depth = 50;

    // Create hitlist (world)
    hit_list world = new_hit_list();

    material mat_ground = {.type = MTYPE_LAMBERTIAN, .color = {0.5f, 0.5f, 0.5f}};
    sphere ground = { .center = {0, -1000.f, 0}, .radius = 1000.0f, .material = &mat_ground };
    hit_list_add_tail(&world, &ground, HTYPE_SPHERE);


    material mat0 = {.type = MTYPE_DIELECTRIC, .n_refract = 1.5};
    material mat1 = {.type = MTYPE_LAMBERTIAN, .color = {0.4f, 0.2f, 0.1f}};
    material mat2 = {.type = MTYPE_METAL, .color = {0.7f, 0.6f, 0.5f}, .fuzziness = 0.0f};

    sphere s[3] = {
        { .center = {0, 0.5f, 0}, .radius = 1.0f, .material = &mat0 },
        { .center = {-4, 0.5f, 0}, .radius = 1.0f, .material = &mat1 },
        { .center = {4, 0.5f, 0}, .radius = 1.0f, .material = &mat2 }
    };

    hit_list_add_tail(&world, &s[0], HTYPE_SPHERE);
    hit_list_add_tail(&world, &s[1], HTYPE_SPHERE);
    hit_list_add_tail(&world, &s[2], HTYPE_SPHERE);

    material mlist[500];
    sphere   slist[500];
    i32 i = 0;

    // Add some random spheres
    for (i32 a = -11; a < 11; a++) 
    {
        for (i32 b = -11; b < 11; b++) 
        {
            f32 choose_mat = random_f32();
            v3_f32 center = {a + 0.9f*random_f32(), 0.2f, b + 0.9f*random_f32()};

            v3_f32 ref = {4, 0.2f, 0};

            if (v3_f32_len(v3_f32_sub(center, ref)) > 0.9f) 
            {
                if (choose_mat < 0.8) 
                {
                    // diffuse
                    v3_f32 albedo = v3_f32_random_range(0, 1);
                    material m = {.type = MTYPE_LAMBERTIAN, .color = albedo};
                    mlist[i] = m;
                    sphere s = {.center = center, .radius = 0.2f, .material = &mlist[i]};
                    slist[i] = s;
                    hit_list_add_tail(&world, &slist[i], HTYPE_SPHERE);
                } 
                else if (choose_mat < 0.95) 
                {
                    // metal
                    v3_f32 albedo = v3_f32_random_range(0.5, 1);
                    f32 fuzz = random_range_f32(0, 0.5);
                    material m = {.type = MTYPE_METAL, .color = albedo, .fuzziness = fuzz};
                    mlist[i] = m;
                    sphere s = {.center = center, .radius = 0.2f, .material = &mlist[i]};
                    slist[i] = s;
                    hit_list_add_tail(&world, &slist[i], HTYPE_SPHERE);
                } 
                else 
                {
                    // glass
                    material m = {.type = MTYPE_DIELECTRIC, .n_refract = 1.5};
                    mlist[i] = m;
                    sphere s = {.center = center, .radius = 0.2f, .material = &mlist[i]};
                    slist[i] = s;
                    hit_list_add_tail(&world, &slist[i], HTYPE_SPHERE);
                }
                i++;
            }
        }
    }

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

    v3_f32 red = {1.0f, 0.0f, 0.0f};
    image_buffer_set_all(image, red);

    // Render
    for(i32 j = 0; j < height; j++)
    {
        printf("%d/%d scanlines completed.\n", j, height);
        for(i32 i = 0; i < width; i++)
        {
            v3_f32 pixel_color = {0, 0, 0};
            for(i32 s = 0; s < spp; s++)
            {
                f32 u = (f32)(i + random_f32()) / (width-1);
                f32 v = (f32)(j + random_f32()) / (height-1);

                ray r = ray_from_cam_info(&cam, u, v);
                
                pixel_color = v3_f32_add(pixel_color, ray_color(&r, &world, max_depth));
            }

            pixel_color = v3_f32_scalar_mult(pixel_color, 1.0f / spp);
            pixel_color = v3_f32_comp_sqrt(pixel_color); // For a gamma of 2.0
            image_buffer_set_pixel(image, i, height - j - 1, pixel_color);
        }
    }

    write_img_buffer_to_file(image, "output.ppm");

    free_hit_list(&world);

    free_image_buffer(image);
}
