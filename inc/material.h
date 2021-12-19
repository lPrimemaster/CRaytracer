#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "common.h"
#include "caster.h"
#include "image.h"

#define TEXTURE_COLOR_UNITS 8

struct texture
{
    v3_f32 color[TEXTURE_COLOR_UNITS];
    u8 type;
    img_buffer* image;
};

#define TTYPE_COLOR        0
#define TTYPE_CHECKER      1
#define TTYPE_IMAGE_ALBEDO 2
#define TTYPE_IMAGE_EMIT   3

// Materials
struct material
{
    i8 type;
    // v3_f32 color;
    texture texture;
    f32 fuzziness; // Metal only
    f32 n_refract; // Dielectric only
};

#define MTYPE_LAMBERTIAN 0
#define MTYPE_METAL      1
#define MTYPE_DIELECTRIC 2
#define MTYPE_DIFF_LIGHT 3

// TODO: Remove this att_color nonsense
i8 ray_scatter_lambertian(ray* r, hit_record* rec, ray* scattered);

i8 ray_scatter_metal(ray* r, hit_record* rec, ray* scattered);

i8 ray_scatter_dielectric(ray* r, hit_record* rec, ray* scattered);

// TODO
i8 ray_emit_emitter(ray* r, hit_record* rec, v3_f32 att_color, ray* scattered);

v2_f32 get_uv_coord(void* hittable, i8 type, v3_f32 p);
v3_f32 texture_get_color_at(texture* tex, v2_f32 uv, v3_f32 p);

#endif
