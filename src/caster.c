#include "../inc/caster.h"
#include "../inc/material.h"
#include "../inc/profiler.h"

v3_f32 ray_at(ray* r, f32 t)
{
    return v3_f32_add(r->o, v3_f32_scalar_mult(r->d, t));
}

void hit_record_set_face(hit_record* rec, const ray* r, const v3_f32 outn)
{
    rec->f = v3_f32_dot(r->d, outn) < 0.0f;
    rec->n = rec->f ? outn : v3_f32_inv(outn);
}

i8 hit_aabb(aabb* b, ray* r, f32 tmin, f32 tmax)
{
    // x
    f32 t_x_a = (b->min.x - r->o.x) / r->d.x;
    f32 t_x_b = (b->max.x - r->o.x) / r->d.x;
    f32 t0_x = fminf(t_x_a, t_x_b);
    f32 t1_x = fmaxf(t_x_a, t_x_b);

    tmin = fmaxf(t0_x, tmin);
    tmax = fminf(t1_x, tmax);
    if(tmax <= tmin) return 0;

    // y
    f32 t_y_a = (b->min.y - r->o.y) / r->d.y;
    f32 t_y_b = (b->max.y - r->o.y) / r->d.y;
    f32 t0_y = fminf(t_y_a, t_y_b);
    f32 t1_y = fmaxf(t_y_a, t_y_b);

    tmin = fmaxf(t0_y, tmin);
    tmax = fminf(t1_y, tmax);
    if(tmax <= tmin) return 0;

    // z
    f32 t_z_a = (b->min.z - r->o.z) / r->d.z;
    f32 t_z_b = (b->max.z - r->o.z) / r->d.z;
    f32 t0_z = fminf(t_z_a, t_z_b);
    f32 t1_z = fmaxf(t_z_a, t_z_b);

    tmin = fmaxf(t0_z, tmin);
    tmax = fminf(t1_z, tmax);
    if(tmax <= tmin) return 0;
    
    return 1;
}

aabb create_aabb(void* hittable, i8 hit_type)
{
    aabb r;

    switch (hit_type)
    {
    case HTYPE_SPHERE:
        sphere* s = (sphere*)hittable;
        v3_f32 rad_vec = {s->radius, s->radius, s->radius};
        r.min = v3_f32_sub(s->center, rad_vec);
        r.max = v3_f32_add(s->center, rad_vec);
        return r;
    case HTYPE_RECT_XY:
        xy_rect* rt = (xy_rect*)hittable;
        f32 half_du = rt->dims.u / 2;
        f32 half_dv = rt->dims.v / 2;
        f32 x0 = rt->center.x - half_du;
        f32 x1 = rt->center.x + half_du;
        f32 y0 = rt->center.y - half_dv;
        f32 y1 = rt->center.y + half_dv;
        r.min.x = x0;
        r.min.y = y0;
        r.min.z = rt->center.z - 0.0001f;
        r.max.x = x1;
        r.max.y = y1;
        r.max.z = rt->center.z + 0.0001f;
        return r;
    }
}

aabb aabb_surrounding_box(aabb b0, aabb b1)
{
    v3_f32 min = {
        fminf(b0.min.x, b1.min.x),
        fminf(b0.min.y, b1.min.y),
        fminf(b0.min.z, b1.min.z)
    };

    v3_f32 max = {
        fmaxf(b0.max.x, b1.max.x),
        fmaxf(b0.max.y, b1.max.y),
        fmaxf(b0.max.z, b1.max.z)
    };

    aabb r = {.min = min, .max = max};
    return r;
}

i8 hit_sphere(sphere* s, ray* r, f32 tmin, f32 tmax, hit_record* rec)
{
    v3_f32 oc = v3_f32_sub(r->o, s->center);
    f32 a = v3_f32_dot(r->d, r->d);
    f32 hb = v3_f32_dot(oc, r->d);
    f32 c = v3_f32_dot(oc, oc) - s->radius * s->radius;
    f32 d = hb*hb - a*c;
    
    if(d < 0)
    {
        return 0;
    }
    
    f32 sqrt_d = sqrtf(d);
    f32 root = (-hb - sqrt_d) / a;
    if(root < tmin || root > tmax)
    {
        root = (-hb + sqrt_d) / a;
        if(root < tmin || root > tmax)
        {
            return 0;
        }
    }

    rec->t = root;
    rec->p = ray_at(r, root);
    v3_f32 outn = v3_f32_scalar_div(v3_f32_sub(rec->p, s->center), s->radius);
    hit_record_set_face(rec, r, outn);
    rec->uv = get_uv_coord(s, HTYPE_SPHERE, outn);
    rec->m = s->material;
    return 1;
}

i8 hit_xy_rect(xy_rect* rt, ray* r, f32 tmin, f32 tmax, hit_record* rec)
{
    f32 t = (rt->center.z - r->o.z) / r->d.z;

    if(t < tmin || t > tmax)
        return 0;
    
    f32 x = r->o.x + t * r->d.x;
    f32 y = r->o.y + t * r->d.y;

    f32 half_du = rt->dims.u / 2;
    f32 half_dv = rt->dims.v / 2;

    f32 x0 = rt->center.x - half_du;
    f32 x1 = rt->center.x + half_du;
    f32 y0 = rt->center.y - half_dv;
    f32 y1 = rt->center.y + half_dv;

    if(x < x0 || x > x1 || y < y0 || y > y1)
        return 0;

    rec->uv.u = (x-x0) / rt->dims.u;
    rec->uv.v = (y-y0) / rt->dims.v;
    rec->t = t;

    v3_f32 outn = {0, 0, 1};

    hit_record_set_face(rec, r, outn);
    rec->m = rt->material;
    rec->p = ray_at(r, t);

    return 1;
}

i8 hit_xz_rect(xz_rect* rt, ray* r, f32 tmin, f32 tmax, hit_record* rec);
i8 hit_yz_rect(yz_rect* rt, ray* r, f32 tmin, f32 tmax, hit_record* rec);

hit_list new_hit_list()
{
    hit_list r;
    r.count = 0;
    r.head = NULL;
    r.types = NULL;
    return r;
}

void hit_list_add_tail(hit_list* list, void* hittable, i8 hit_type)
{
    if(list->head == NULL)
    {
        list->head = (void**)malloc(sizeof(void*));
        list->types = (i8*)malloc(sizeof(i8));
        *(list->head) = hittable;
        *(list->types) = hit_type;
    }
    else
    {
        list->head = realloc(list->head, sizeof(void*) * (list->count + 1));
        list->types = (i8*)realloc(list->types, sizeof(i8) * (list->count + 1));
        list->head[list->count] = hittable;
        list->types[list->count] = hit_type;
    }

    list->count++;
}

i8 hit_list_hit_all(hit_list* list, ray* r, f32 tmin, f32 tmax, hit_record* rec)
{
    hit_record tmp_rec;
    i8 hit_smt = 0;
    f32 closest = tmax;

    for(i32 i = 0; i < list->count; i++)
    {
        switch (*(list->types + i))
        {
        case HTYPE_SPHERE:
            if(hit_sphere((sphere*)(*(list->head + i)), r, tmin, closest, &tmp_rec))
            {
                hit_smt = 1;
                closest = tmp_rec.t;
                *rec = tmp_rec;
            }
            break;
        case HTYPE_RECT_XY:
            if(hit_xy_rect((xy_rect*)(*(list->head + i)), r, tmin, closest, &tmp_rec))
            {
                hit_smt = 1;
                closest = tmp_rec.t;
                *rec = tmp_rec;
            }
            break;
        case HTYPE_BVH:
            if(bvh_tree_hit_all((bvh_node*)(*(list->head + i)), r, tmin, closest, &tmp_rec))
            {
                hit_smt = 1;
                closest = tmp_rec.t;
                *rec = tmp_rec;
            }
            break;
        }
    }

    return hit_smt;
}

void free_hit_list(hit_list* list)
{
    free(list->head);
    free(list->types);
    list->head = NULL;
    list->types = NULL;
    list->count = 0;
}

aabb hit_list_create_aabb(hit_list* list)
{
    aabb r;
    i8 first_box = 0;

    for(i32 i = 0; i < list->count; i++)
    {
        aabb obj_box = create_aabb(list->head[i], list->types[i]);
        r = first_box ? first_box = 0, obj_box : 
            aabb_surrounding_box(r, obj_box);
    }
    return r;
}

static void swap_list(hit_list* ptr, i32 i, i32 j)
{
    void* tmp_head = ptr->head[i];
    i8    tmp_type = ptr->types[i];

    ptr->head[i] = ptr->head[j];
    ptr->types[i] = ptr->types[j];

    ptr->head[j] = tmp_head;
    ptr->types[j] = tmp_type;
}

static i8 box_compare(void* h0, void* h1, i8 t0, i8 t1, i32 axis)
{
    aabb box0 = create_aabb(h0, t0);
    aabb box1 = create_aabb(h1, t1);

    return box0.min.p[axis] < box1.min.p[axis];
}

static i32 qsort_partition(hit_list* ptr, i32 start, i32 stop, i32 axis)
{
    void* pivot = ptr->head[stop];

    i32 i = start - 1;

    for(i32 j = start; j <= stop - 1; j++)
    {
        if(box_compare(ptr->head[j], pivot, ptr->types[j], ptr->types[stop], axis))
        {
            i++;
            swap_list(ptr, i, j);
        }
    }

    swap_list(ptr, i+1, stop);
    return i + 1;
}

static void sort_hit_list(hit_list* ptr, i32 start, i32 stop, i32 axis)
{
    if(start < stop)
    {
        i32 pi = qsort_partition(ptr, start, stop, axis);

        sort_hit_list(ptr, start, pi - 1, axis);
        sort_hit_list(ptr, pi + 1, stop, axis);
    }
}

bvh_node* new_bvh_tree(hit_list* list, i32 start, i32 stop)
{
    bvh_node* parent = (bvh_node*)malloc(sizeof(bvh_node));
    i32 axis = random_range_i32(0, 2);
    i32 count = stop - start;
    i8 left_type = HTYPE_BVH;
    i8 right_type = HTYPE_BVH;

    if(count == 1)
    {
        parent->left = parent->right = NULL;
        parent->hittable_left = parent->hittable_right = *list->head;
        parent->hittable_left_type = parent->hittable_right_type = *list->types;
        left_type = right_type = *list->types;
    }
    else if(count == 2)
    {
        parent->left = parent->right = NULL;

        void* a = *(list->head + start);
        i8 at = *(list->types + start);
        void* b = *(list->head + start + 1);
        i8 bt = *(list->types + start + 1);
        if(box_compare(a, b, at, bt, axis))
        {
            parent->hittable_left = a;
            parent->hittable_right = b;

            parent->hittable_left_type = at;
            parent->hittable_right_type = bt;

            left_type = at;
            right_type = bt;
        }
        else
        {
            parent->hittable_left = b;
            parent->hittable_right = a;

            parent->hittable_left_type = bt;
            parent->hittable_right_type = at;

            left_type = bt;
            right_type = at;
        }
    }
    else
    {
        parent->hittable_left = parent->hittable_right = NULL;

        // TODO: Use some optimized sort later on
        sort_hit_list(list, start, stop, axis);

        i32 mid = start + count / 2;
        parent->left  = new_bvh_tree(list, start, mid);
        parent->right = new_bvh_tree(list, mid, stop);
    }

    aabb bleft  = create_aabb(parent->left  ? parent->left  : parent->hittable_left,   left_type);
    aabb bright = create_aabb(parent->right ? parent->right : parent->hittable_right, right_type);

    parent->box = aabb_surrounding_box(bleft, bright);

    return parent;
}

void free_bvh_tree(bvh_node* parent)
{
    if(parent)
    {
        if(parent->left && parent->right)
        {
            free_bvh_tree(parent->left);
            free_bvh_tree(parent->right);
        }
        free(parent);
    }
}

i8 bvh_tree_hit_all(bvh_node* parent, ray* r, f32 tmin, f32 tmax, hit_record* rec)
{
    // No hit
    if(!hit_aabb(&parent->box, r, tmin, tmax))
    {
        return 0;
    }

    // Is not final
    if(parent->left != NULL)
    {
        i8 hit_left = bvh_tree_hit_all(parent->left, r, tmin, tmax, rec);
        i8 hit_right = bvh_tree_hit_all(parent->right, r, tmin, hit_left ? rec->t : tmax, rec);
        return hit_left || hit_right;
    }

    // Is final
    switch (parent->hittable_left_type)
    {
    case HTYPE_SPHERE:
        return hit_sphere((sphere*)(parent->hittable_left), r, tmin, tmax, rec);
    case HTYPE_RECT_XY:
        return hit_xy_rect((xy_rect*)(parent->hittable_left), r, tmin, tmax, rec);
    }

    // TODO: Use a flag not to run this if there is only a single hittable
    switch (parent->hittable_right_type)
    {
    case HTYPE_SPHERE:
        return hit_sphere((sphere*)(parent->hittable_right), r, tmin, tmax, rec);
    case HTYPE_RECT_XY:
        return hit_xy_rect((xy_rect*)(parent->hittable_left), r, tmin, tmax, rec);
    }
}

v3_f32 ray_color(ray* r, hit_list* list, i32 depth, i32* total_rays)
{
    PROFILE_START;
    static const v3_f32 sc = {0, 0, -1};
    static const v3_f32 white = { 1.0f, 1.0f, 1.0f };
    static const v3_f32 black = { 0.0f, 0.0f, 0.0f };
    static const v3_f32 blueish = { 0.1f, 0.1f, 0.1f };

    hit_record rec;


    if(depth <= 0)
        return black;
        
    *total_rays += 1;

    if(hit_list_hit_all(list, r, 0.001f, 0xFFFFFF, &rec))
    {
        ray scattered;

        switch (rec.m->type)
        {
        case MTYPE_LAMBERTIAN:
            if(ray_scatter_lambertian(r, &rec, &scattered))
            {
                v3_f32 color = texture_get_color_at(&rec.m->texture, rec.uv, rec.p);
                return v3_f32_mult(color, ray_color(&scattered, list, depth - 1, total_rays));
            }
            break;
        case MTYPE_METAL:
            if(ray_scatter_metal(r, &rec, &scattered))
            {
                v3_f32 color = texture_get_color_at(&rec.m->texture, rec.uv, rec.p);
                return v3_f32_mult(color, ray_color(&scattered, list, depth - 1, total_rays));
            }
            break;
        case MTYPE_DIELECTRIC:
            if(ray_scatter_dielectric(r, &rec, &scattered))
            {
                return v3_f32_mult(white, ray_color(&scattered, list, depth - 1, total_rays));
            }
            break;
        case MTYPE_DIFF_LIGHT:
            // TODO: Assume no scatter for now
            // If it scatters return emitted + attenuation * ray_color(scattered, background, world, depth-1);
            // Instead
            switch (rec.m->texture.type)
            {
            case TTYPE_COLOR:
                return rec.m->texture.color[0];
            case TTYPE_IMAGE_ALBEDO:
                return texture_get_color_at(&rec.m->texture, rec.uv, rec.p);
            }
        }
        return black;
    }

    return black;
    
    // NOTE: This is a return for colored backgrounds with diffuse light
    // v3_f32 u_dir = v3_f32_to_unit(r->d);
    // f32 t = 0.5f * (u_dir.y + 1.0f);


    // return v3_f32_add(v3_f32_scalar_mult(white, (1.0f - t)), v3_f32_scalar_mult(blueish, t));
    PROFILE_END;
}
