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
    char        kval; /* current value of K. */
    block_t*    succ; /* successor block_t in list. */
    char        data[];
};

int init_pool();
void* b_malloc(size_t size); 
void b_free(void* ptr);
void pop_block(block_t* block, int k);
//block_t* get_block(size_t size);
void split_block(int k);
void merge_block(block_t* block);
block_t* get_block(int k);

static block_t* freelist[N + 1] = {NULL};
static char* mem_pool;

int init_pool() 
{
    mem_pool = sbrk(POOL_SIZE);

    if (mem_pool == (void*) -1)
        return -1;

    freelist[N] = mem_pool;
    block_t* init_mem = (block_t*) mem_pool;
    init_mem->reserved = 0;
    init_mem->kval = N;
    init_mem->succ = NULL;
    return 1;
}



void* b_malloc(size_t size) 
{
    int         k;
    block_t*    ptr;

    if (size <= 0)
        return NULL;

    if (mem_pool == NULL) {
        if (!init_pool())
            return NULL;
    }

    k = 0;
    while ((1 << k) < (size + BLOCK_SIZE))
        k++;

    ptr = get_block(k);
    if (ptr == NULL)
        return NULL;

    return ptr->data;
}

void b_free(void* ptr)
{
    block_t*    block;
    int         k;
    char*       char_ptr;

    printf("Enter free\n");
    char_ptr = (char*) ptr - BLOCK_SIZE;
    if (char_ptr == NULL)
        return;

    block = (block_t*) char_ptr;
    k = block->kval;
    printf("Past block finder k = %d\n", k);
    if (!block->reserved)
        return;

    block->reserved = 0;
    block->succ = freelist[k];
    freelist[k] = block;

    // Dubbelkolla det här.
    merge_block(block);


}

void merge_block(block_t* block)
{
    block_t*    buddy;
    block_t*    start;
    int         k;

    k = block->kval;
    //printf("    block->kval: %d\n", block->kval);
    buddy = (char*) mem_pool + (((char*) block - (char*) mem_pool) ^ (1 << k));

    if (buddy->reserved || (buddy->kval != k)){
        printf("Hmm, buddy->reserved = %d, buddy->kval = %d, k = %d\n", buddy->reserved, buddy->kval, k);
        return;
    }

    if ((char*) block > (char*) buddy)
        start = buddy;
    else
        start = block;

    pop_block(buddy, k);
    pop_block(block, k);

    start->kval = k + 1;
    start->succ = freelist[k + 1];
    freelist[k + 1] = start;
    
    block->reserved = 0;
    buddy->reserved = 0;


    if (k < N - 1) {
        merge_block(start);
    }
}

void pop_block(block_t* block, int k)
{
    block_t* p = freelist[k];
    if (p == block) {
        freelist[k] = p->succ;
    } else {
        while (p->succ != block)
            p = p->succ;
        p->succ = block->succ;
        // EVENTUELLT KOLLA NULL 
    }
}

block_t* get_block(int k)
{
    int         i;
    int         j;
    block_t*    block;

    j = k;

    while (!freelist[j] && j <=N)
        j++;

    //printf("Found j: %d\n", j);
    for (i = j; i > k; i--) {
        split_block(i);
    }

    block = freelist[k];
    freelist[k] = freelist[k]->succ;
    block->succ = NULL;
    block->reserved = 1;
    return block;
}

/*block_t* get_block(size_t size)
{
    int         k;
    int         j;
    block_t*    block;

    k = get_k(size + BLOCK_SIZE);
    if (freelist[k] == NULL)
        split_block(k + 1);

    block = freelist[k];
    freelist[k] = freelist[k]->succ;
    block->succ = NULL;
    block->reserved = 1;

    return block;
}*/



int get_k(size_t size) 
{
    int k = 0;

    while (size > (1 << k))
        k++;
    return k;
}

void split_block(int k)
{
    block_t    *base, *sp1, *sp2;
    char*       ptr;       

    base = freelist[k];
    freelist[k] = freelist[k]->succ;

    ptr = ((char*) base) + (1 << (k - 1));
    sp1 = base;
    sp2 = (block_t*) ptr;

    freelist[k - 1] = sp1;

    sp1->succ = sp2;
    sp2->succ = NULL;

    sp1->kval = k - 1;
    sp2->kval = k - 1;
    //printf("    sp1->kval: %d\n", sp1->kval);
    //printf("    sp2->kval: %d\n", sp2->kval);
}


int main() 
{
    char* ptr1 = b_malloc(450);
    b_free(ptr1);
    return 0;
}
