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

#define BLOCK_SIZE  sizeof(block)

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

int find_k(size_t size)
{
    int k = 0;
    printf("Total size: %zu\n", size);
    while(size > (1 << k))
        k++;

    return k;
}

void* find_mem(int k)
{
    int     j;
    int     i;
    block*  first;
    block*  base_block;
    block*  block_1;
    block*  block_2;
    

    j = k;
    while (!freelist[j] && j <= N)
        j++;

    printf("Found J: %d\n", j);

    first = freelist[j];
    if (!first)
        return NULL;
    /* First J with available block found */
    /* Time for splitting */
    printf("Split shit!\n");
    for (i = j; i >= k; i--){
        base_block = freelist[i];
        
        printf("Split started for I: %d\n", i);
        block_1 = (block*) base_block;
        block_2 = (block*) ((char*) base_block + (1 << (i - 1)));
        printf("Two blocks found.\n");

        block_1->reserved = 0;
        block_2->reserved = 0;
        printf("Blocks seems to be working.");

        block_1->succ = block_1->pred = block_2;
        block_2->succ = block_2->pred = block_1;
        printf("Assigning preds\n");
        block_1->kval = i - 1;
        block_2->kval = i - 1;

        printf("Sets vals\n");
        freelist[i - 1] = block_1;
        remove_freeitem(base_block);
    }

    return freelist[k];
}

void remove_freeitem(block* block_t)
{
    block*  temp;
    int     k;

    k = block_t->kval;
    if (block_t == freelist[k])
        freelist[k] = block_t->succ;
    temp = block_t->pred;
    temp->succ = block_t->succ;
    block_t->succ->pred = temp;
    block_t->reserved = 1;
}

void merge(block* block_t)
{

}

void* b_malloc(size_t size) 
{
    int     check_pool;
    int     k;
    block*  memory;
    block*  last;

    if (size <= 0)
        return NULL;

    if (!mem_pool) {
        check_pool = init_pool();
        if (!check_pool)
            return NULL;
    }

    k = find_k(size + BLOCK_SIZE);
    memory = find_mem(k);
    if (!memory)
        return NULL;
    /* Remove space from freelist */
    remove_freeitem(memory);
    
    return memory;
}

void b_free(void* ptr)
{
    block*  block_t;
    block*  buddy;
    int     k;
    
    if (!mem_pool)
        return;

    block_t = (block*) ((char*) ptr - BLOCK_SIZE);
    if (!block_t)
        return;

    k = block_t->kval;
    buddy = (block*) ((char*) mem_pool + (((char*) block_t - (char*) mem_pool) ^ (1 << k)));
    if(!buddy)
        return;

    if (!buddy->reserved){
        // Let the merge begin.
        remo

        b_free(PEKAREN);
    } else {
        block_t->reserved = 0;
        /* Add last in freelist.
        Maybe could put in more optimal position. */
        block_t->pred = freelist[k]->pred;
        block_t->succ = freelist[k];
        freelist[k]->pred->succ = block_t;
        freelist[k]->pred = block_t;
    }
}



int main() 
{
    block* mem = b_malloc(127);
    block* two = b_malloc(22);
    
    return 0;
}