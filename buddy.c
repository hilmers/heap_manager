#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define N (12)
#define POOL_SIZE (1 << N) // Equal to 2^N.

typedef struct block block;
struct block {
    unsigned reserved:1; /* one if reserved. */
    char kval; /* current value of K. */
    block* succ; /* successor block in list. */
    block* pred; /* predecessor block in list. */
};

static block* freelist[N + 1] = {NULL};
static block* mem_pool = NULL;

int init_pool() 
{
    mem_pool = sbrk(POOL_SIZE);

    if (mem_pool == (void*) -1)
        return -1;

    mem_pool->reserved = 0;
    mem_pool->kval = N;
    mem_pool->succ = mem_pool->pred = mem_pool;
    freelist[N] = mem_pool;

    return 1;
}

int find_K(size_t size)
{
    int K = 0;
    while(size > (1 << K))
        K++;

    return K;
}



int main() 
{
    printf("K = %d\n", find_K(129));
    return 0;
}