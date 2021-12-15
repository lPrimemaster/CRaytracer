#include "../inc/caster.h"

v3_f32 ray_at(ray* r, f32 t)
{
    return v3_f32_add(r->o, v3_f32_scalar_mult(r->d, t));
}

void hit_record_set_face(hit_record* rec, const ray* r, const v3_f32 outn)
{
    rec->f = v3_f32_dot(r->d, outn) < 0.0f;
    rec->n = rec->f ? outn : v3_f32_inv(outn);
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
    rec->m = s->material;
    return 1;
}

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
