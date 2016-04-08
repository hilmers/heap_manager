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

    while (freelist[j] == NULL)
        j++;

    block = freelist[j];
    freelist[j] = freelist[j]->succ; // Resten är NULL so far ju.

    for (i = j - 1; i > k; i--) {
        block->k -= 1;
        //printf("block->k: %d\t i: %zu\n", block->k, i);
        buddy = (void *)(mem_pool + (((char *)block - mem_pool) ^ (1 << i)));
        buddy->k = i;
        buddy->reserved = 0;
        buddy->succ = freelist[i]; // BEHÖVS DENNA? 
        freelist[i] = buddy;
    }

    block->reserved = 1;
    return block;
}

void *b_malloc(size_t size)
{
    size_t      k;
    block_t     *ptr;

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
    while ((1 << k) < (size + BLOCK_SIZE))
        k++;

    ptr = split_blocks(k);
    return ptr->data;

}

void b_free(void *ptr)
{
    block_t     *block, *buddy;
    int         k;

    block = (char*) ptr - BLOCK_SIZE;
    buddy = (char *)(mem_pool + (((char *)block - mem_pool) ^ (1 << block->k)));


    if (buddy->reserved || (buddy->k != k)){
        printf("Hmm, buddy->reserved = %d, buddy->kval = %d, k = %d\n", buddy->reserved, buddy->kval, k);
        return;
    }

    block->reserved = 0;



}


int main() 
{
    char* ptr1 = b_malloc(450);
    char* ptr2 = b_malloc(22);
    char* ptr3 = b_malloc(700);
    char* ptr4 = b_malloc(300);
    b_free(ptr1);
    b_free(ptr2);
    char* ptr5 = b_malloc(200);
    b_free(ptr3);
    b_free(ptr4);

    b_free(ptr5);
    return 0;
}
