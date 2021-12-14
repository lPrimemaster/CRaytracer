#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

typedef float f32;
typedef unsigned char u8;
typedef unsigned int  u32;
typedef signed   char i8;
typedef          int  i32;

// Random generators
void init_random()
{
    time_t t;
    srand((unsigned)time(&t));
}

f32 random_f32()
{
    return rand() / (RAND_MAX + 1.0f);
}

f32 random_range_f32(f32 min, f32 max)
{
    return min + (max - min) * random_f32();
}

f32 clamp(f32 x, f32 min, f32 max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

// Vector3 code
typedef struct {
  f32 x;  
  f32 y;  
  f32 z;  
} v3_f32;

#define v3_f32_SCALAR_OP(name, op) \
v3_f32 v3_f32_scalar_##name(v3_f32 v, f32 s) \
{ \
    v3_f32 r; \
    r.x = v.x op s; \
    r.y = v.y op s; \
    r.z = v.z op s; \
    return r; \
} \

v3_f32_SCALAR_OP(add, +)
v3_f32_SCALAR_OP(sub, -)
v3_f32_SCALAR_OP(mult, *)
v3_f32_SCALAR_OP(div, /)

v3_f32 v3_f32_inv(v3_f32 v)
{
    v3_f32 r;
    r.x = -v.x;
    r.y = -v.y;
    r.z = -v.z;
    return r;
}

v3_f32 v3_f32_add(v3_f32 a, v3_f32 b)
{
    // TODO: Could use SIMD
    v3_f32 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}

v3_f32 v3_f32_sub(v3_f32 a, v3_f32 b)
{
    // TODO: Could use SIMD
    v3_f32 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    r.z = a.z - b.z;
    return r;
}

v3_f32 v3_f32_cross(v3_f32 a, v3_f32 b)
{
    // TODO: Could use SIMD
    v3_f32 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

f32 v3_f32_dot(v3_f32 a, v3_f32 b)
{
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

f32 v3_f32_len(v3_f32 v)
{
    return sqrtf(v3_f32_dot(v, v));
}

v3_f32 v3_f32_to_unit(v3_f32 v)
{
    return v3_f32_scalar_div(v, v3_f32_len(v));
}

v3_f32 v3_f32_comp_sqrt(v3_f32 v)
{
    v3_f32 r;
    r.x = sqrtf(v.x);
    r.y = sqrtf(v.y);
    r.z = sqrtf(v.z);
    return r;
}

v3_f32 v3_f32_random_range(f32 min, f32 max)
{
    v3_f32 r;
    r.x = random_range_f32(min, max);
    r.y = random_range_f32(min, max);
    r.z = random_range_f32(min, max);
    return r;
}

// TODO: Random unit sphere is dumb this way. Why not use spherical coordinates for this
v3_f32 v3_f32_random_unit_sphere()
{
    while(1)
    {
        v3_f32 point = v3_f32_random_range(-1, 1);
        if(v3_f32_dot(point, point) < 1)
            return point;
    }
}

v3_f32 v3_f32_random_hemisphere(v3_f32 n) {
    v3_f32 usph = v3_f32_random_unit_sphere();
    if (v3_f32_dot(usph, n) > 0.0) // In the same hemisphere as the normal
        return usph;
    else
        return v3_f32_inv(usph);
}


// Ray code
typedef struct
{
    v3_f32 o;
    v3_f32 d;
} ray;

v3_f32 ray_at(ray* r, f32 t)
{
    return v3_f32_add(r->o, v3_f32_scalar_mult(r->d, t));
}

typedef struct
{
    v3_f32 p; // Hit point
    v3_f32 n; // Hit normal
    f32 t; // Hit root
    i8 f; // Hit face (front/back)
} hit_record;

void hit_record_set_face(hit_record* rec, const ray* r, const v3_f32 outn)
{
    rec->f = v3_f32_dot(r->d, outn) < 0.0f;
    rec->n = rec->f ? outn : v3_f32_inv(outn);
}

typedef struct
{
    v3_f32 center;
    f32 radius;
} sphere;

i8 hit_sphere(sphere* s, ray* r, f32 tmin, f32 tmax, hit_record* rec)
{
    v3_f32 oc = v3_f32_sub(r->o, s->center);
    f32 a = v3_f32_dot(r->d, r->d);
    f32 hb = v3_f32_dot(oc, r->d);
    f32 c = v3_f32_dot(oc, oc) - s->radius * s->radius;
    f32 d = hb*hb - a*c;
    
    if(d < 0)
    {
        return 0;
    }
    
    f32 sqrt_d = sqrtf(d);
    f32 root = (-hb - sqrt_d) / a;
    if(root < tmin || root > tmax)
    {
        root = (-hb + sqrt_d) / a;
        if(root < tmin || root > tmax)
        {
            return 0;
        }
    }

    rec->t = root;
    rec->p = ray_at(r, root);
    v3_f32 outn = v3_f32_scalar_div(v3_f32_sub(rec->p, s->center), s->radius);
    hit_record_set_face(rec, r, outn);
    return 1;
}

typedef struct
{
    void** head;
    i8* types;
    i32 count;
} hit_list;

#define HTYPE_SPHERE 0

hit_list new_hit_list()
{
    hit_list r;
    r.count = 0;
    r.head = NULL;
    r.types = NULL;
    return r;
}

void hit_list_add_tail(hit_list* list, void* hittable, i8 hit_type)
{
    if(list->head == NULL)
    {
        list->head = (void**)malloc(sizeof(void*));
        list->types = (i8*)malloc(sizeof(i8));
        *(list->head) = hittable;
        *(list->types) = hit_type;
    }
    else
    {
        list->head = realloc(list->head, sizeof(void*) * (list->count + 1));
        list->types = (i8*)realloc(list->types, sizeof(i8) * (list->count + 1));
        list->head[list->count] = hittable;
        list->types[list->count] = hit_type;
    }

    list->count++;
}

i8 hit_list_hit_all(hit_list* list, ray* r, f32 tmin, f32 tmax, hit_record* rec)
{
    hit_record tmp_rec;
    i8 hit_smt = 0;
    f32 closest = tmax;

    for(i32 i = 0; i < list->count; i++)
    {
        switch (*(list->types + i))
        {
        case HTYPE_SPHERE:
            if(hit_sphere((sphere*)(*(list->head + i)), r, tmin, closest, &tmp_rec))
            {
                hit_smt = 1;
                closest = tmp_rec.t;
                *rec = tmp_rec;
            }
            break;
        }
    }

    return hit_smt;
}

void free_hit_list(hit_list* list)
{
    free(list->head);
    free(list->types);
    list->head = NULL;
    list->types = NULL;
    list->count = 0;
}

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
        v3_f32 ruh = v3_f32_random_hemisphere(rec.n);
        v3_f32 target = v3_f32_add(rec.p, ruh);

        ray nray = {.o = rec.p, .d = v3_f32_sub(target, rec.p)};
        v3_f32 nit = ray_color(&nray, list, depth - 1);

        return v3_f32_scalar_mult(nit, 0.5f);
    }
    
    v3_f32 u_dir = v3_f32_to_unit(r->d);
    f32 t = 0.5f * (u_dir.y + 1.0f);


    return v3_f32_add(v3_f32_scalar_mult(white, (1.0f - t)), v3_f32_scalar_mult(blueish, t));
}

// Image buffer
typedef struct
{
    u8* data;
    u32 w;
    u32 h;
} img_buffer;

// Asume rgb for now (TODO: Check malloc return)
img_buffer* new_image_buffer(u32 w, u32 h)
{
    img_buffer* b = (img_buffer*)malloc(sizeof(img_buffer));
    b->w = w;
    b->h = h;
    b->data = malloc(w * h * 3 * sizeof(u8));
    return b;
}

void image_buffer_set_all(img_buffer* buffer, v3_f32 color)
{
    if(buffer != NULL)
    {
        for(u32 i = 0; i < buffer->w * buffer->h * 3; i += 3)
        {
            *(buffer->data + i    ) = (u8)(color.x * 255);
            *(buffer->data + i + 1) = (u8)(color.y * 255);
            *(buffer->data + i + 2) = (u8)(color.z * 255);
        }
    }
}

void image_buffer_set_pixel(img_buffer* buffer, i32 x, i32 y, v3_f32 color)
{
    buffer->data[3 * y * buffer->w + 3 * x    ] = (u8)(clamp(color.x, 0.0f, 0.999f) * 256);
    buffer->data[3 * y * buffer->w + 3 * x + 1] = (u8)(clamp(color.y, 0.0f, 0.999f) * 256);
    buffer->data[3 * y * buffer->w + 3 * x + 2] = (u8)(clamp(color.z, 0.0f, 0.999f) * 256);
}

void free_image_buffer(img_buffer* buffer)
{
    if(buffer != NULL)
    {
        free(buffer->data);
        free(buffer);
        buffer = NULL;
    }
}

void write_img_buffer_to_file(img_buffer* buffer, const char* filename)
{
    if(buffer != NULL)
    {
        FILE* f = fopen(filename, "wb");
        fprintf(f, "P6\n%d %d\n255\n", buffer->w, buffer->h);
        fwrite(buffer->data, 3 * sizeof(u8), buffer->h * buffer->w, f);
        fclose(f);
    }
}

int main(int argc, char* arg[])
{
    init_random();
    
    // Viewport / Image
    const f32 aspect = 16.0f / 9.0f;
    const i32 width = 400;
    const i32 height = (i32)(400 / aspect);
    const i32 spp = 16;
    const i32 max_depth = 50;

    // Create hitlist (world)
    hit_list world = new_hit_list();

    sphere s = { .center = {0, 0, -1}, .radius = 0.5f };
    hit_list_add_tail(&world, &s, HTYPE_SPHERE);

    sphere s2 = { .center = {0, -100.5f, -1}, .radius = 100.0f };
    hit_list_add_tail(&world, &s2, HTYPE_SPHERE);

    // Camera
    f32 viewport_h = 2.0f;
    f32 viewport_w = aspect * viewport_h;
    v3_f32 focal_len = {0, 0, 1.0f};

    v3_f32 origin = {0, 0, 0};
    v3_f32 half_horizontal = {viewport_w / 2, 0, 0};
    v3_f32 half_vertical = {0, viewport_h / 2, 0};
    v3_f32 horizontal = {viewport_w, 0, 0};
    v3_f32 vertical = {0, viewport_h, 0};

    v3_f32 llcorner      = v3_f32_sub(v3_f32_sub(v3_f32_sub(origin, half_horizontal), half_vertical), focal_len);

    // Setup image buffer
    img_buffer* image = new_image_buffer(width, height);

    v3_f32 red = {1.0f, 0.0f, 0.0f};
    image_buffer_set_all(image, red);

    // Render
    for(i32 j = height-1; j >= 0; j--)
    {
        for(i32 i = 0; i < width; i++)
        {
            v3_f32 pixel_color = {0, 0, 0};
            for(i32 s = 0; s < spp; s++)
            {
                f32 u = (f32)(i + random_f32()) / (width-1);
                f32 v = (f32)(j + random_f32()) / (height-1);

                ray r;
                r.o = origin;
                r.d = v3_f32_add(llcorner, v3_f32_add(v3_f32_scalar_mult(horizontal, u), v3_f32_sub(v3_f32_scalar_mult(vertical, v), origin)));
                pixel_color = v3_f32_add(pixel_color, ray_color(&r, &world, max_depth));
            }

            pixel_color = v3_f32_scalar_mult(pixel_color, 1.0f / spp);
            pixel_color = v3_f32_comp_sqrt(pixel_color); // For a gamma of 2.0
            image_buffer_set_pixel(image, i, j, pixel_color);
        }
    }

    write_img_buffer_to_file(image, "output.ppm");

    free_hit_list(&world);

    free_image_buffer(image);
}
