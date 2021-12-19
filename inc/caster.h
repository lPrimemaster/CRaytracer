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
typedef struct texture texture;

// Records
typedef struct
{
    v3_f32 p; // Hit point
    v3_f32 n; // Hit normal
    f32 t; // Hit root
    i8 f; // Hit face (front/back)
    material* m; // Hit material type
    v2_f32 uv; // Texcoord
} hit_record;

void hit_record_set_face(hit_record* rec, const ray* r, const v3_f32 outn);

typedef struct
{
    v3_f32 min, max;
} aabb;

i8 hit_aabb(aabb* b, ray* r, f32 tmin, f32 tmax);
aabb create_aabb(void* hittable, i8 hit_type);
aabb aabb_surrounding_box(aabb b0, aabb b1);

typedef struct
{
    v3_f32 center;
    f32 radius;
    material* material;
} sphere;

struct rect
{
    v3_f32 center;
    v2_f32 dims;
    material* material;
};

typedef struct rect xy_rect;
typedef struct rect xz_rect;
typedef struct rect yz_rect;

// Raycaster tester
i8 hit_sphere(sphere* s, ray* r, f32 tmin, f32 tmax, hit_record* rec);

i8 hit_xy_rect(xy_rect* rt, ray* r, f32 tmin, f32 tmax, hit_record* rec);
i8 hit_xz_rect(xz_rect* rt, ray* r, f32 tmin, f32 tmax, hit_record* rec);
i8 hit_yz_rect(yz_rect* rt, ray* r, f32 tmin, f32 tmax, hit_record* rec);

typedef struct
{
    void** head;
    i8* types;
    i32 count;
} hit_list;

#define HTYPE_SPHERE  0
#define HTYPE_RECT_XY 1
#define HTYPE_RECT_XZ 2
#define HTYPE_RECT_YZ 3
#define HTYPE_BVH     0xF

hit_list new_hit_list();
void free_hit_list(hit_list* list);
void hit_list_add_tail(hit_list* list, void* hittable, i8 hit_type);
i8 hit_list_hit_all(hit_list* list, ray* r, f32 tmin, f32 tmax, hit_record* rec);
aabb hit_list_create_aabb(hit_list* list);

typedef struct _bvh_node
{
    aabb box;
    struct _bvh_node* left;
    struct _bvh_node* right;
    void* hittable_left;
    void* hittable_right;
    i8 hittable_left_type;
    i8 hittable_right_type;
} bvh_node;

bvh_node* new_bvh_tree(hit_list* list, i32 start, i32 stop);
void free_bvh_tree(bvh_node* parent);
i8 bvh_tree_hit_all(bvh_node* parent, ray* r, f32 tmin, f32 tmax, hit_record* rec);

v3_f32 ray_color(ray* r, hit_list* list, i32 depth);
#endif
