#ifndef _CASTER_H_
#define _CASTER_H_
#include "common.h"

// Rays
typedef struct
{
    v3_f32 o;
    v3_f32 d;
} ray;

v3_f32 ray_at(ray* r, f32 t);

typedef struct material material;

// Records
typedef struct
{
    v3_f32 p; // Hit point
    v3_f32 n; // Hit normal
    f32 t; // Hit root
    i8 f; // Hit face (front/back)
    material* m; // Hit material type
} hit_record;

void hit_record_set_face(hit_record* rec, const ray* r, const v3_f32 outn);

typedef struct
{
    v3_f32 center;
    f32 radius;
    material* material;
} sphere;

// Raycaster tester
i8 hit_sphere(sphere* s, ray* r, f32 tmin, f32 tmax, hit_record* rec);

typedef struct
{
    void** head;
    i8* types;
    i32 count;
} hit_list;

#define HTYPE_SPHERE 0

hit_list new_hit_list();
void free_hit_list(hit_list* list);
void hit_list_add_tail(hit_list* list, void* hittable, i8 hit_type);
i8 hit_list_hit_all(hit_list* list, ray* r, f32 tmin, f32 tmax, hit_record* rec);

v3_f32 ray_color(ray* r, hit_list* list, i32 depth);
#endif
