#include "../inc/material.h"

i8 ray_scatter_lambertian(ray* r, hit_record* rec, ray* scattered)
{
    scattered->o = rec->p;
    scattered->d = v3_f32_add(rec->n, v3_f32_random_unit_norm());

    if (v3_f32_near_zero(scattered->d))
        scattered->d = rec->n;

    return 1;
}

i8 ray_scatter_metal(ray* r, hit_record* rec, ray* scattered)
{
    scattered->o = rec->p;
    scattered->d = v3_f32_add(v3_f32_reflect(v3_f32_to_unit(r->d), rec->n), v3_f32_scalar_mult(v3_f32_random_unit_sphere(), rec->m->fuzziness));
    return (v3_f32_dot(scattered->d, rec->n) > 0);
}

i8 ray_scatter_dielectric(ray* r, hit_record* rec, ray* scattered)
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

v2_f32 get_uv_coord(void* hittable, i8 type, v3_f32 p)
{
    switch (type)
    {
    case HTYPE_SPHERE:
        v2_f32 uv = {
            .u = (atan2f(-p.z, p.x) + PI) / (2 * PI), 
            .v = acosf(-p.y) / PI
        };
        return uv;
    }
}

static v2_f32 clamp_uv(v2_f32 uv)
{
    uv.u = clamp(uv.u, 0.0f, 1.0f);
    uv.v = clamp(uv.v, 0.0f, 1.0f);
    return uv;
}

v3_f32 texture_get_color_at(texture* tex, v2_f32 uv, v3_f32 p)
{
    switch (tex->type)
    {
    case TTYPE_COLOR:
        return tex->color[0];
    case TTYPE_CHECKER:
        {
            f32 sines = sinf(10 * p.x) * sinf(10 * p.y) * sinf(10 * p.z);
            if (sines < 0)
                return tex->color[0];
            else
                return tex->color[1];
        }
    case TTYPE_IMAGE_ALBEDO:
        {
            // TODO: Change mode from various, not only clamp
            uv = clamp_uv(uv);
            u32 i = (u32)(uv.u * tex->image->w);
            u32 j = (u32)(uv.v * tex->image->h);

            if(i >= tex->image->w) i = tex->image->w - 1;
            if(j >= tex->image->h) j = tex->image->h - 1;

            static f32 color_scale = 1.0f / 255.0f;
            // TODO: Create a new var for bpp * w
            u8* pix = tex->image->data + j * tex->image->w * tex->image->bpp + i * tex->image->bpp;
            v3_f32 color = {
                color_scale * pix[0],
                color_scale * pix[1],
                color_scale * pix[2]
            };

            return color;
        }
    }
}
