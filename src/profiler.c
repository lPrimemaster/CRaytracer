# include "../inc/profiler.h"

static i32 global_max_id  = 0;
static i8  global_dry_run = 1;
static struct nlist* hashtab[HASHSIZE];

/* hash: form hash value for string s */
static unsigned hash(char* s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

/* lookup: look for s in hashtab */
struct nlist* lookup(char* s)
{
    struct nlist* np;
    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
          return np; /* found */
    return NULL; /* not found */
}

profile_block* lookup_direct(char* s)
{
    struct nlist* nl = lookup(s);
    if(nl == NULL)
        return NULL;
    return nl->defn;
}

char* sdup(char* s);
profile_block* pbdup(profile_block* p);

struct nlist* insert(char* name, profile_block* defn)
{
    struct nlist* np;
    unsigned hashval;
    if ((np = lookup(name)) == NULL) { /* not found */
        np = (struct nlist*) malloc(sizeof(*np));
        if (np == NULL || (np->name = sdup(name)) == NULL)
          return NULL;
        hashval = hash(name);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else /* already there */
        free((void*) np->defn); /*free previous defn */
    if ((np->defn = pbdup(defn)) == NULL)
       return NULL;
    return np;
}

char* sdup(char* s) /* make a duplicate of s */
{
    char* p;
    p = (char*) malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL)
       strcpy(p, s);
    return p;
}

profile_block* pbdup(profile_block* p)
{
    profile_block* r = (profile_block*)malloc(sizeof(profile_block));
    if(r != NULL)
    {
        r->block_line_start = p->block_line_start;
        r->block_line_end   = p->block_line_end;
        r->cycles           = p->cycles;
        strcpy(r->function, p->function);
    }
    return r;
}

void free_hash_table()
{
    for(i32 i = 0; i < HASHSIZE; i++)
    {
        struct nlist* p;
        if((p = hashtab[i]) != NULL)
        {
            free(p->name);
            free(p->defn);
            free(p);
        }
    }
}

void _profile_block_start(char* function, char* file, i32 ls)
{
    char hmap_key[256];
    strcpy(hmap_key, function);
    strcat(hmap_key, file);
    profile_block* pb = lookup_direct(hmap_key);
    if(pb != NULL)
    {
        pb->block_line_start = ls;
        pb->cycles = __rdtsc();
    }
    else
    {
        profile_block pb;
        pb.block_line_start = ls;
        pb.cycles = __rdtsc();
        strcpy(pb.function, function);
        insert(hmap_key, &pb);
    }
}

void _profile_block_end(char* function, char* file, i32 le)
{
    char hmap_key[256];
    strcpy(hmap_key, function);
    strcat(hmap_key, file);
    profile_block* pb = lookup_direct(hmap_key);

    pb->block_line_end = le;
    pb->cycles = __rdtsc() - pb->cycles;
}