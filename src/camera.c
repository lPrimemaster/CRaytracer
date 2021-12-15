#include "../inc/camera.h"

cam_info calculate_cam_info(v3_f32 cam_pos, v3_f32 look_at, v3_f32 cam_up, f32 cam_fov, f32 cam_ar, f32 aperture, f32 focus)
{
    cam_info ci;
    f32 theta = cam_fov * 3.141592654f / 180.0f;
    f32 h = tanf(theta/2.0f);
    f32 viewport_height = 2.0f * h;
    f32 viewport_width = cam_ar * viewport_height;

    v3_f32 w = v3_f32_to_unit(v3_f32_sub(cam_pos, look_at));
    v3_f32 u = v3_f32_to_unit(v3_f32_cross(cam_up, w));
    v3_f32 v = v3_f32_cross(w, u);

    ci.u0 = u;
    ci.v0 = v;

    ci.o = cam_pos;
    ci.h = v3_f32_scalar_mult(u, focus * viewport_width);
    ci.v = v3_f32_scalar_mult(v, focus * viewport_height);

    v3_f32 hh = v3_f32_scalar_div(ci.h, 2);
    v3_f32 hv = v3_f32_scalar_div(ci.v, 2);
    
    ci.llc = v3_f32_sub(v3_f32_sub(v3_f32_sub(ci.o, hh), hv), v3_f32_scalar_mult(w, focus));

    ci.lr = aperture / 2.0f;

    return ci;
}

ray ray_from_cam_info(cam_info* ci, f32 u, f32 v)
{
    ray r;

    v3_f32 rd = v3_f32_scalar_mult(v3_f32_random_unit_disk(), ci->lr);
    v3_f32 offset = v3_f32_add(v3_f32_scalar_mult(ci->u0, rd.x), v3_f32_scalar_mult(ci->v0, rd.y));

    r.o = v3_f32_add(ci->o, offset);
    r.d = v3_f32_add(ci->llc, v3_f32_add(v3_f32_scalar_mult(ci->h, u), v3_f32_sub(v3_f32_sub(v3_f32_scalar_mult(ci->v, v), ci->o), offset)));
    return r;
}
