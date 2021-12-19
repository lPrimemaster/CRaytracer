#ifndef _IMAGE_H_
#define _IMAGE_H_
#include "common.h"

// Image buffer
typedef struct
{
    u8* data;
    u32 w;
    u32 h;
    u8 bpp;
} img_buffer;

// Asume rgb for now (TODO: Check malloc return)
img_buffer* new_image_buffer(u32 w, u32 h, u8 bpp);
void image_buffer_set_all(img_buffer* buffer, v3_f32 color);
void image_buffer_set_pixel(img_buffer* buffer, i32 x, i32 y, v3_f32 color);
void image_buffer_add_pixel(img_buffer* buffer, i32 x, i32 y, v3_f32 color);
v3_f32 image_buffer_get_pixel(img_buffer* buffer, i32 x, i32 y);
void free_image_buffer(img_buffer* buffer);
void write_img_buffer_to_file(img_buffer* buffer, const char* filename);

img_buffer* read_bitmap_image(const char* filename);

#endif