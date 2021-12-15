#include "../inc/common.h"

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

v3_f32 v3_f32_mult(v3_f32 a, v3_f32 b)
{
    v3_f32 r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    r.z = a.z * b.z;
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

v3_f32 v3_f32_random_unit_disk()
{
    while(1)
    {
        v3_f32 point = {random_range_f32(-1, 1), random_range_f32(-1, 1), 0};
        if(v3_f32_dot(point, point) < 1)
            return point;
    }
}

v3_f32 v3_f32_random_unit_norm()
{
    return v3_f32_to_unit(v3_f32_random_unit_sphere());
}

v3_f32 v3_f32_random_hemisphere(v3_f32 n) 
{
    v3_f32 usph = v3_f32_random_unit_sphere();
    if (v3_f32_dot(usph, n) > 0.0) // In the same hemisphere as the normal
        return usph;
    else
        return v3_f32_inv(usph);
}

i8 v3_f32_near_zero(v3_f32 v) 
{
    const f32 s = 1E-8f;
    return (fabs(v.x) < s) && (fabs(v.y) < s) && (fabs(v.z) < s);
}

v3_f32 v3_f32_reflect(v3_f32 v, v3_f32 n)
{
    return v3_f32_sub(v, v3_f32_scalar_mult(n, 2 * v3_f32_dot(v,n)));
}

v3_f32 v3_f32_refract(v3_f32 unit_v, v3_f32 n, f32 refract_material_ratio, f32 cos_theta)
{
    v3_f32 ro_orto = v3_f32_scalar_mult(v3_f32_add(unit_v, v3_f32_scalar_mult(n, cos_theta)), refract_material_ratio);
    v3_f32 ro_para = v3_f32_scalar_mult(n, -sqrtf(fabsf(1.0f - v3_f32_dot(ro_orto, ro_orto))));
    return v3_f32_add(ro_orto, ro_para);
}

f32 reflectance(f32 cos, f32 refract_material_ratio)
{
    f32 r0 = (1 - refract_material_ratio) / (1 + refract_material_ratio);
    r0 = r0 * r0;
    return r0 + (1 - r0) * powf(1 - cos, 5);
}
