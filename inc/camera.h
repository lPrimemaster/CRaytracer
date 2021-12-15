#ifndef _CAMERA_H_
#define _CAMERA_H_
#include "common.h"
#include "caster.h"

typedef struct
{
    v3_f32 o;
    v3_f32 llc;
    v3_f32 h, v;
    v3_f32 u0, v0;
    f32 lr;
} cam_info;

cam_info calculate_cam_info(v3_f32 cam_pos, v3_f32 look_at, v3_f32 cam_up, f32 cam_fov, f32 cam_ar, f32 aperture, f32 focus);
ray ray_from_cam_info(cam_info* ci, f32 u, f32 v);
#endif