#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define N (26)
#define POOL_SIZE (1 << N) // Equal to 2^N.
#define BLOCK_SIZE  sizeof(block_t)

typedef struct block_t block_t;

struct block_t {
    unsigned    reserved:1; /* one if reserved. */
    char        k; /* current value of K. */
    block_t*    succ; /* successor block_t in list. */
    char        data[];
};

static block_t* freelist[N + 1] = {0};
static char* mem_pool;



block_t *split_blocks(size_t k)
{
    size_t      i;
    size_t      j;
    block_t     *block, *buddy;

    j = k + 1;

    while (freelist[j] == NULL) {
        j++;
    }

    block = freelist[j];
    freelist[j] = freelist[j]->succ; // Resten är NULL so far ju.

    for (i = j - 1; i > k; i--) {
        block->k -= 1;
        buddy = (void *)(mem_pool + (((char *)block - mem_pool) ^ (1 << i)));
        buddy->k = i;
        buddy->reserved = 0;
        buddy->succ = freelist[i];
        freelist[i] = buddy;
    }

    block->reserved = 1;
    return block;
}

void *malloc(size_t size)
{
    size_t      k;
    block_t     *ptr;
    //fprintf(stderr, "malloc with size: %d\n", size);

    if (mem_pool == NULL) {
        mem_pool = sbrk(POOL_SIZE);

        if (mem_pool == (void*) -1)
            return NULL;

        freelist[N] = mem_pool;
        block_t *mem_first = (block_t*) mem_pool;
        mem_first->reserved = 0;
        mem_first->k = N;
        mem_first->succ = NULL;    
    }

    k = 0;
    while ((1 << k) < (size + BLOCK_SIZE)) {
        k++;
    }

    ptr = split_blocks(k);
    return ptr->data;

}

void pop_block(block_t* block)
{
    block_t* p = freelist[block->k];
    if (p == block) {
        freelist[block->k] = p->succ;
        return;
    } 
    while (p->succ != block){
        p = p->succ;
    }
    p->succ = block->succ; 
}


void free(void *ptr)
{
    block_t     *block, *buddy;
    int         k;

    if (ptr == NULL)
        return;

    block = (char*) ptr - BLOCK_SIZE;
    buddy = (char *)(mem_pool + (((char *)block - mem_pool) ^ (1 << block->k)));
    //fprintf(stderr, "free with k: %d\n", block->k);
    block->reserved = 0;

    while (block->k < N && buddy->reserved == 0 && buddy->k == block->k) {
        pop_block(buddy);
        
        if (block > buddy) {
            block_t *temp = block;
            block = buddy;
            buddy = temp;
        }

        block->k += 1;
        buddy = (char *)(mem_pool + (((char *)block - mem_pool) ^ (1 << block->k)));
    }
    block->succ = freelist[block->k];
    freelist[block->k] = block;
}

void* calloc(size_t n, size_t size)
{
    size_t  tot_size;
    void*   memory;

    tot_size = n * size;
    memory = malloc(tot_size);
    if (!memory)
        return NULL;
    memset(memory, 0, tot_size);
    return memory;
}

void* realloc(void* memory, size_t size)
{
    block_t*  old_block;
    void*   new_memory;
    int     k;

    if (!memory)
        return malloc(size);

    old_block = (block_t*) ((char*) memory - BLOCK_SIZE);
    
    k = 0;
    while ((1 << k) < (size + BLOCK_SIZE))
        k++;

    if (old_block->k >= k)
        return memory;

    new_memory = malloc(size);
    if (!new_memory)
        return NULL;

    memcpy(new_memory, memory, 1 << old_block->k);
    free(memory);
    return new_memory;
}