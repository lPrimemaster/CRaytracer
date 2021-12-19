#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <math.h>
#include <windows.h>

#define PI 3.141592654f

CRITICAL_SECTION CriticalSection;

typedef float              f32;
typedef unsigned char      u8;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef signed   char      i8;
typedef          int       i32;

// Random generators
void init_random();
f32 random_f32();
f32 random_range_f32(f32 min, f32 max);

i32 random_range_i32(i32 min, i32 max);

f32 clamp(f32 x, f32 min, f32 max);

// Vector2 code
typedef struct {
  union
  {
    struct
    {
      f32 u;
      f32 v;
    };
    f32 p[2];
  };
} v2_f32;

// Vector3 code
typedef struct {
  union
  {
    struct
    {
      f32 x;  
      f32 y;  
      f32 z; 
    };
    f32 p[3];
  };
} v3_f32;

#define v3_f32_SCALAR_OP_D(name) \
v3_f32 v3_f32_scalar_##name(v3_f32 v, f32 s)

#define v3_f32_SCALAR_OP(name, op) \
v3_f32_SCALAR_OP_D(name) \
{ \
    v3_f32 r; \
    r.x = v.x op s; \
    r.y = v.y op s; \
    r.z = v.z op s; \
    return r; \
} \

v3_f32_SCALAR_OP_D(add);
v3_f32_SCALAR_OP_D(sub);
v3_f32_SCALAR_OP_D(mult);
v3_f32_SCALAR_OP_D(div);

v3_f32 v3_f32_inv(v3_f32 v);
v3_f32 v3_f32_add(v3_f32 a, v3_f32 b);
v3_f32 v3_f32_sub(v3_f32 a, v3_f32 b);
v3_f32 v3_f32_mult(v3_f32 a, v3_f32 b);
v3_f32 v3_f32_cross(v3_f32 a, v3_f32 b);
f32 v3_f32_dot(v3_f32 a, v3_f32 b);
f32 v3_f32_len(v3_f32 v);
v3_f32 v3_f32_to_unit(v3_f32 v);
v3_f32 v3_f32_comp_sqrt(v3_f32 v);

v3_f32 v3_f32_random_range(f32 min, f32 max);
// TODO: Random unit sphere is dumb this way. Why not use spherical coordinates for this
v3_f32 v3_f32_random_unit_sphere();
v3_f32 v3_f32_random_unit_disk();
v3_f32 v3_f32_random_unit_norm();
v3_f32 v3_f32_random_hemisphere(v3_f32 n);

i8 v3_f32_near_zero(v3_f32 v);
v3_f32 v3_f32_reflect(v3_f32 v, v3_f32 n);
v3_f32 v3_f32_refract(v3_f32 unit_v, v3_f32 n, f32 refract_material_ratio, f32 cos_theta);
f32 reflectance(f32 cos, f32 refract_material_ratio);

#endif