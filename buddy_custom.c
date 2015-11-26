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
    printf("find mem entered\n");
    j = k;
    while (!freelist[j] && j <= N)
        j++;
    printf("found great j: %d\n", j);
    first = freelist[j];
    if (!first)
        return NULL;
    /* First J with available block found */
    /* Time for splitting */
    for (i = j; i >= k; i--){
        printf("split for i: %d\n", i);
        base_block = freelist[i];
        block_1 = (block*) base_block;
        block_2 = (block*) ((char*) base_block + (1 << (i - 1)));
        block_1->kval = i - 1;
        block_2->kval = i - 1;
        printf("before remove_freeitem\n");
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
    if (block_t == block_t->succ) {
        printf("freelist[%d] = null\n", k);
        freelist[k] = NULL;
        return;
    }
    if (block_t == freelist[k]){
        printf("freelist[%d] == block_t\n", k);
        freelist[k] = block_t->succ;
    }
    printf("set reserved\n");
    temp = block_t->pred;
    printf("temp is alive\n");
    temp->succ = block_t->succ;
    printf("found succ\n");
    (block_t->succ)->pred = temp;
    if (k == N)
        mem_pool = NULL;
}

void add_freeitem(block* block_t)
{
    int     k;
    block*  first;

    printf("Entered add_freeitem\n");

    k = block_t->kval;
    block_t->reserved = 0;
    if (!freelist[k]) {
        /* Freelist empty for that k */
        block_t->succ = block_t->pred = block_t;
        freelist[k] = block_t;
    } else {
        /* Add block_t first in circular freelist */
        printf("freelist[%d] not empty\n", k);
        first = freelist[k];
        block_t->succ = first;
        block_t->pred = first->pred;
        printf("block now in place\n");
        (first->pred)->succ = block_t;
        printf("preds succ now set\n");
        first->pred = block_t;
        freelist[k] = block_t;
        printf("done i add_freeitem\n");
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
        printf("Create new mem_pool\n");
        check_pool = init_pool();
        if (!check_pool)
            return NULL;
    }
    printf("Trying to find k and memory\n");
    k = find_k(size + BLOCK_SIZE);
    printf("k found-> %d\n", k);
    memory = find_mem(k);
    printf("Memory in malloc found\n");
    if (!memory)
        return NULL;
    /* Remove space from freelist */
    remove_freeitem(memory);
    return (char*) memory + BLOCK_SIZE;
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
    /* Base case for checking if we reached top. E.g. memory completly empty */
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
        /* Recursivly merge blocks */
        b_free((char*) start + BLOCK_SIZE);
    } else {
        add_freeitem(block_t);
    }
}

void* b_calloc(size_t n, size_t size)
{
    size_t  tot_size;
    void*   memory;

    printf("Enters calloc\n");
    tot_size = n * size;
    memory = b_malloc(tot_size);

    if (!memory)
        return NULL;
    printf("Found memory\n");

    memset(memory, 0, tot_size);
    printf("Memset went fine. \n");
    return memory;
}

void* b_realloc(void* memory, size_t size)
{
    block*  old_block;
    void*   new_memory;
    int     k;

    if (!memory)
        return b_malloc(size);

    old_block = (block*) ((char*) memory - BLOCK_SIZE);
    k = find_k(size);
    if (old_block->kval >= k)
        return memory;

    new_memory = b_malloc(size);
    if (!new_memory)
        return NULL;

    memcpy(new_memory, memory, 1 << old_block->kval);
    b_free(memory);
    return new_memory;
}

int main() 
{
    void* mem = b_malloc(127);
    void* two = b_malloc(22);
    void* three = b_malloc(22);
    void* s2 = b_malloc(22);
    void* s3 = b_malloc(4000);
    //void* s4 = b_calloc(20, 20);
    //void* s4 = b_malloc(20*20);
    printf("Done with calloc\n");
    void* s5 = b_malloc(22);
    printf("Done with malloc after calloc\n");
    void* s6 = b_malloc(600);
    void* s7 = b_malloc(22);
    void* s8 = b_malloc(22);

    printf("Malloced both\n");
    b_free(two);
    printf("22 freed\n");
    b_realloc(mem, 400);
    printf("127 freed\n");
    b_free(three);
    b_free(s2);
    b_free(s3);
    b_free(s4);
    b_free(s5);
    b_free(s6);
    b_free(s7);
    b_free(s8);
    b_free(mem);

    
    return 0;
}
