#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "common.h"
#include "caster.h"

// Materials
struct material
{
    i8 type;
    v3_f32 color;
    f32 fuzziness; // Metal only
    f32 n_refract; // Dielectric only
};

#define MTYPE_LAMBERTIAN 0
#define MTYPE_METAL      1
#define MTYPE_DIELECTRIC 2
#define MTYPE_EMMITER    3

// TODO: Remove this att_color nonsense
i8 ray_scatter_lambertian(ray* r, hit_record* rec, v3_f32 att_color, ray* scattered);

i8 ray_scatter_metal(ray* r, hit_record* rec, v3_f32 att_color, ray* scattered);

i8 ray_scatter_dielectric(ray* r, hit_record* rec, v3_f32 att_color, ray* scattered);

// TODO
i8 ray_emit_emitter(ray* r, hit_record* rec, v3_f32 att_color, ray* scattered);
#endif
