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
    printf("Total size: %zu\n", size);
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

        printf("Assigning preds\n");
        block_1->kval = i - 1;
        block_2->kval = i - 1;


        remove_freeitem(base_block);
        printf("Removed base-block\n");

        add_freeitem(block_1);
        add_freeitem(block_2);
        printf("Blocks added. \n");
    }

    return freelist[k];
}

void remove_freeitem(block* block_t)
{
    block*  temp;
    int     k;

    k = block_t->kval;
    block_t->reserved = 1;
    printf("    remove_freeitem: K found, now block reserved. k: %d\n", k);
    if (!block_t->pred || !block_t->succ)
        printf("    PROBLEMAS\n");
    temp = block_t->pred;
    temp->succ = block_t->succ;
    block_t->succ->pred = temp;
    printf("    remove_freeitem: Block loose in linked list.\n");
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
    printf("    Add: Added in freelist for k: %d\n", k);
    if (!freelist[k]) {
        printf("    Add: freelist empty.\n");
        block_t->succ = block_t->pred = block_t;
        freelist[k] = block_t;
    } else {
        printf("    Add: freelist not empty\n");
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
    
    return memory + BLOCK_SIZE;
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

    printf("Free: Block found.\n");
    k = block_t->kval;
    buddy = (block*) ((char*) mem_pool + (((char*) block_t - (char*) mem_pool) ^ (1 << k)));
    if(!buddy)
        return;

    printf("Free: Buddy found.\n");
    if (!buddy->reserved){
        printf("Free: Buddy not reserved, begin merge.\n");
        block* start;
        if ((char*) block_t > (char*) buddy)
            start = buddy;
        else
            start = block_t;
        printf("Free: Found start, k: %d\n", k);
        remove_freeitem(block_t);
        remove_freeitem(buddy);
        printf("Free: Block & Buddy removed.\n");
        start->kval = k + 1;
        add_freeitem(start);
        printf("Free: Added one level above.\n");
        b_free(start + BLOCK_SIZE);
    } else {
        printf("Free: Buddy reserved, remove block from freelist.\n");
        add_freeitem(block_t);
    }
}



int main() 
{
    block* mem = b_malloc(127);
    //block* two = b_malloc(22);
    b_free(mem);
    
    return 0;
}