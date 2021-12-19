#ifndef _PROFILER_H_
#define _PROFILER_H_
#include "common.h"

// The profiler structure, holding timing data
typedef struct
{
    u64 cycles;
    char function[128];
    i32 block_line_start;
    i32 block_line_end;
} profile_block;

void _profile_block_start(char* function, char* file, i32 ls);
void _profile_block_end  (char* function, char* file, i32 le);

#define PROFILE

#ifdef PROFILE
#define PROFILE_START _profile_block_start(__FUNCTION__, __FILE__, __LINE__)
#define PROFILE_END   _profile_block_end  (__FUNCTION__, __FILE__, __LINE__)
#else
#define PROFILE_START
#define PROFILE_END
#endif

// A simple hash table for string keys
struct nlist {
    struct nlist* next;
    char* name;
    profile_block* defn;
};

#define HASHSIZE 101

struct nlist* lookup(char* s);
struct nlist* insert(char* name, profile_block* defn);
void free_hash_table();

#endif