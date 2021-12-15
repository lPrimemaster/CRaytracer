#include "../inc/image.h"
#include "../inc/common.h"

// Image buffer
struct img_buffer
{
    u8* data;
    u32 w;
    u32 h;
};

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