#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef float f32;
typedef unsigned char u8;
typedef unsigned int  u32;
typedef          int  i32;

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

f32 v3_f32_len(v3_f32 a)
{
    return sqrtf(v3_f32_dot(a, a));
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

v3_f32 ray_color(ray* r)
{
    v3_f32 u_dir = v3_f32_scalar_div(r->d, v3_f32_len(r->d));
    f32 t = 0.5f * (u_dir.y + 1.0f);

    static const v3_f32 white = { 1.0f, 1.0f, 1.0f };
    static const v3_f32 blueish = { 0.5f, 0.7f, 1.0f };

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
    buffer->data[3 * y * buffer->w + 3 * x    ] = (u8)(color.x * 255);
    buffer->data[3 * y * buffer->w + 3 * x + 1] = (u8)(color.y * 255);
    buffer->data[3 * y * buffer->w + 3 * x + 2] = (u8)(color.z * 255);
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
    // Viewport / Image
    const f32 aspect = 16.0f / 9.0f;
    const i32 width = 400;
    const i32 height = (i32)(400 / aspect);

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
            f32 u = (f32)i / (width-1);
            f32 v = (f32)j / (height-1);

            ray r;
            r.o = origin;
            r.d = v3_f32_add(llcorner, v3_f32_add(v3_f32_scalar_mult(horizontal, u), v3_f32_sub(v3_f32_scalar_mult(vertical, v), origin)));

            v3_f32 color = ray_color(&r);
            image_buffer_set_pixel(image, i, j, color);
        }
    }

    write_img_buffer_to_file(image, "output.ppm");

    free_image_buffer(image);
}
