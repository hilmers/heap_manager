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

int init_pool();
int find_k(size_t size);
block* find_mem(int k);
void remove_freeitem(block* block_t);
void add_freeitem(block* block_t);
void* b_malloc(size_t size); 
void b_free(void* ptr);

static block* freelist[N + 1] = {NULL};
static block* mem_pool = NULL;

int init_pool() 
{
    mem_pool = sbrk(POOL_SIZE);

    if (mem_pool == (void*) -1)
        return -1;

    mem_pool->kval = N;
    add_freeitem(mem_pool);

    return 1;
}

int find_k(size_t size)
{
    int k = 0;
    while(size > (1 << k))
        k++;

    return k;
}

block* find_mem(int k)
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

    first = freelist[j];
    if (!first)
        return NULL;
    /* First J with available block found */
    /* Time for splitting */
    for (i = j; i >= k; i--){
        base_block = freelist[i];
        
        block_1 = (block*) base_block;
        block_2 = (block*) ((char*) base_block + (1 << (i - 1)));

        block_1->kval = i - 1;
        block_2->kval = i - 1;


        remove_freeitem(base_block);

        add_freeitem(block_1);
        add_freeitem(block_2);
    }

    return freelist[k];
}

void remove_freeitem(block* block_t)
{
    block*  temp;
    int     k;

    k = block_t->kval;
    block_t->reserved = 1;
    temp = block_t->pred;
    temp->succ = block_t->succ;
    block_t->succ->pred = temp;
    if (block_t == freelist[k]){
        freelist[k] = block_t->succ;
        if (block_t == block_t->succ)
            freelist[k] = NULL;
    }
    
}

void add_freeitem(block* block_t)
{
    int     k;

    k = block_t->kval;
    block_t->reserved = 0;
    if (!freelist[k]) {
        block_t->succ = block_t->pred = block_t;
        freelist[k] = block_t;
    } else {
        freelist[k]->pred->succ = block_t;
        block_t->pred = freelist[k]->pred;
        block_t->succ = freelist[k];
        freelist[k]->pred = block_t;
    }
}

void* b_malloc(size_t size) 
{
    int     check_pool;
    int     k;
    block*  memory;

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
    return (char*) memory + sizeof(block);
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

    /* Base case for checking if we reached top. */
    if (k >= N)
        return;

    buddy = (block*) ((char*) mem_pool + (((char*) block_t - (char*) mem_pool) ^ (1 << k)));
    if(!buddy)
        return;

    if (!buddy->reserved){
        block* start;
        if ((char*) block_t > (char*) buddy)
            start = buddy;
        else
            start = block_t;

        remove_freeitem(block_t);
        remove_freeitem(buddy);

        start->kval = k + 1;
        add_freeitem(start);
        b_free((char*) start + BLOCK_SIZE);
    } else {
        add_freeitem(block_t);
    }
}



int main() 
{
    void* mem = b_malloc(127);
    void* two = b_malloc(22);
    printf("Malloced both\n");
    b_free(two);
    printf("22 freed\n");
    b_free(mem);
    printf("127 freed\n");

    
    return 0;
}