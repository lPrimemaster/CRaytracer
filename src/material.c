#include "../inc/material.h"

// TODO: Remove this att_color nonsense
i8 ray_scatter_lambertian(ray* r, hit_record* rec, v3_f32 att_color, ray* scattered)
{
    scattered->o = rec->p;
    scattered->d = v3_f32_add(rec->n, v3_f32_random_unit_norm());

    if (v3_f32_near_zero(scattered->d))
        scattered->d = rec->n;

    return 1;
}

i8 ray_scatter_metal(ray* r, hit_record* rec, v3_f32 att_color, ray* scattered)
{
    scattered->o = rec->p;
    scattered->d = v3_f32_add(v3_f32_reflect(v3_f32_to_unit(r->d), rec->n), v3_f32_scalar_mult(v3_f32_random_unit_sphere(), rec->m->fuzziness));
    return (v3_f32_dot(scattered->d, rec->n) > 0);
}

i8 ray_scatter_dielectric(ray* r, hit_record* rec, v3_f32 att_color, ray* scattered)
{
    f32 rratio = rec->f ? (1.0f / rec->m->n_refract) : rec->m->n_refract;
    scattered->o = rec->p;

    v3_f32 udir = v3_f32_to_unit(r->d);
    v3_f32 iudir = v3_f32_inv(v3_f32_to_unit(r->d));

    f32 cos_theta = fminf(v3_f32_dot(iudir, rec->n), 1.0);
    f32 sin_theta = sqrtf(1.0f - cos_theta * cos_theta);

    i8 not_refract = rratio * sin_theta > 1.0f;

    if(not_refract || reflectance(cos_theta, rratio) > random_f32())
    {
        scattered->d = v3_f32_reflect(udir, rec->n);
    }
    else
    {
        scattered->d = v3_f32_refract(udir, rec->n, rratio, cos_theta);
    }
    return 1;
}