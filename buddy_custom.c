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
        printf("block->k: %d\t i: %zu\n", block->k, i);
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



void pop_block(block_t* block)
{
    block_t* p = freelist[block->k];
    if (p == block) {
        freelist[block->k] = p->succ;
        return;
    } 
    while (p->succ != block)
        p = p->succ;
    p->succ = block->succ; 
}

void b_free(void *ptr)
{
    block_t     *block, *buddy;
    int         k;

    block = (char*) ptr - BLOCK_SIZE;
    buddy = (char *)(mem_pool + (((char *)block - mem_pool) ^ (1 << block->k)));

    printf("Enter free with k: %d\n", block->k);

    block->reserved = 0;

    while (block->k < N && buddy->reserved == 0 && buddy->k == block->k) {
        pop_block(buddy);
        printf("block->k: %d\n", block->k);
        if (block > buddy) {
            block_t *tmp = block;
            block = buddy;
            buddy = block;
        }
        block->k += 1;
        buddy = (char *)(mem_pool + (((char *)block - mem_pool) ^ (1 << block->k)));
    }

    printf("Done! k: %d\n", block->k);
    block->succ = freelist[block->k];
    freelist[block->k] = block;
}

void* b_calloc(size_t n, size_t size)
{
    size_t  tot_size;
    void*   memory;

    tot_size = n * size;
    memory = b_malloc(tot_size);
    if (!memory)
        return NULL;
    memset(memory, 0, tot_size);
    return memory;
}

void* b_realloc(void* memory, size_t size)
{
    block_t*  old_block;
    void*   new_memory;
    int     k;

    if (!memory)
        return b_malloc(size);

    old_block = (block_t*) ((char*) memory - BLOCK_SIZE);
    
    k = 0;
    while ((1 << k) < (size + BLOCK_SIZE))
        k++;

    if (old_block->k >= k)
        return memory;

    new_memory = b_malloc(size);
    if (!new_memory)
        return NULL;

    memcpy(new_memory, memory, 1 << old_block->k);
    b_free(memory);
    return new_memory;
}

int main() 
{
    /*printf("Malloc 1\n");
    char* ptr1 = b_malloc(450);

    printf("\n\nMalloc 2\n");
    char* ptr2 = b_malloc(22);

    printf("\n\nMalloc 3\n");
    char* ptr3 = b_malloc(700);

    printf("\n\nMalloc 4\n");
    char* ptr4 = b_malloc(300);

    printf("\n\nFree 1\n");
    b_free(ptr1);

    printf("\n\nFree 2\n");
    b_free(ptr2);

    printf("\n\nMalloc 5\n");
    char* ptr5 = b_malloc(200);

    printf("\n\nFree 3\n");
    b_free(ptr3);

    printf("\n\nFree 4\n");
    b_free(ptr4);

    printf("\n\nCalloc 1\n");
    char* ptr6 = b_calloc(1, 1022);

    printf("\n\Realloc 1\n");
    char* ptr7 = b_realloc(ptr5, 400);

    printf("\n\nFree 5\n");
    b_free(ptr7);

    b_free(ptr6);*/
     char* ptr9  = b_calloc(1, 363);
 char* ptr10 = b_calloc(1, 22);
 char* ptr11 = b_calloc(1, 1907);
 char* ptr12 = b_calloc(1, 143);
 char* ptr13 = b_calloc(1, 1035);
 char* ptr14 = b_calloc(1, 1344);
 char* ptr15 = b_calloc(1, 615);
 char* ptr16 = b_calloc(1, 1576);
 char* ptr17 = b_calloc(1, 249);
 char* ptr18 = b_calloc(1, 1986);
 char* ptr19 = b_calloc(1, 1840);
 char* ptr20 = b_calloc(1, 580);
 char* ptr21 = b_calloc(1, 1328);
 char* ptr22 = b_calloc(1, 2044);
 char* ptr23 = b_calloc(1, 1782);
 char* ptr24 = b_calloc(1, 1751);
 char* ptr25 = b_calloc(1, 157);
 char* ptr26 = b_calloc(1, 826);
 char* ptr27 = b_calloc(1, 1538);
 char* ptr28 = b_calloc(1, 1053);
 char* ptr29 = b_calloc(1, 435);
 char* ptr30 = b_calloc(1, 296);
 char* ptr31 = b_calloc(1, 1587);
 char* ptr32 = b_calloc(1, 276);
 char* ptr33 = b_calloc(1, 1202);
 char* ptr34 = b_calloc(1, 1360);
 char* ptr35 = b_calloc(1, 1677);
 char* ptr36 = b_calloc(1, 643);
 char* ptr37 = b_calloc(1, 202);
 char* ptr38 = b_calloc(1, 263);
 char* ptr39 = b_calloc(1, 982);
 char* ptr40 = b_calloc(1, 1546);
 char* ptr41 = b_calloc(1, 1117);
 char* ptr42 = b_calloc(1, 1561);
 char* ptr43 = b_calloc(1, 392);
 char* ptr44 = b_calloc(1, 166);
 char* ptr45 = b_calloc(1, 699);
 char* ptr46 = b_calloc(1, 1637);
 char* ptr47 = b_calloc(1, 1348);
 char* ptr48 = b_calloc(1, 921);
 char* ptr49 = b_calloc(1, 808);
 char* ptr50 = b_calloc(1, 1422);
 char* ptr51 = b_calloc(1, 1675);
 char* ptr52 = b_calloc(1, 1376);
 char* ptr53 = b_calloc(1, 273);
 char* ptr54 = b_calloc(1, 1252);
 char* ptr55 = b_calloc(1, 813);
 char* ptr56 = b_calloc(1, 1527);
 char* ptr57 = b_calloc(1, 1772);
 char* ptr58 = b_calloc(1, 794);
 char* ptr59 = b_calloc(1, 636);
 char* ptr60 = b_calloc(1, 1808);
 char* ptr61 = b_calloc(1, 1095);
 char* ptr62 = b_calloc(1, 547);
 char* ptr63 = b_calloc(1, 585);
 char* ptr64 = b_calloc(1, 927);
 char* ptr65 = b_calloc(1, 1743);
 char* ptr66 = b_calloc(1, 438);
 char* ptr67 = b_calloc(1, 1807);
 char* ptr68 = b_calloc(1, 1090);
 char* ptr69 = b_calloc(1, 727);
 char* ptr70 = b_calloc(1, 1115);
 char* ptr71 = b_calloc(1, 728);
 char* ptr72 = b_calloc(1, 405);
 char* ptr73 = b_calloc(1, 1383);
 char* ptr74 = b_calloc(1, 581);
 char* ptr75 = b_calloc(1, 10);
 char* ptr76 = b_calloc(1, 861);
 char* ptr77 = b_calloc(1, 894);
 char* ptr78 = b_calloc(1, 1907);
 char* ptr781 = b_realloc(ptr78, 2000);
 char* ptr79 = b_calloc(1, 1635);
 char* ptr80 = b_calloc(1, 1316);
 char* ptr81 = b_calloc(1, 1248);
 char* ptr82 = b_calloc(1, 909);
 char* ptr83 = b_calloc(1, 46);
 char* ptr84 = b_calloc(1, 103);
 char* ptr85 = b_calloc(1, 186);
 char* ptr86 = b_calloc(1, 905);
 char* ptr87 = b_calloc(1, 1118);
 char* ptr88 = b_calloc(1, 630);
 char* ptr89 = b_calloc(1, 527);
 char* ptr90 = b_calloc(1, 218);
 char* ptr91 = b_calloc(1, 1690);
 char* ptr92 = b_calloc(1, 2040);
 char* ptr93 = b_calloc(1, 1882);
 char* ptr94 = b_calloc(1, 1697);
 char* ptr95 = b_calloc(1, 1097);
 char* ptr96 = b_calloc(1, 1189);
 char* ptr97 = b_calloc(1, 1634);
 char* ptr98 = b_calloc(1, 1230);
 char* ptr99 = b_calloc(1, 780);
 char* ptr100 = b_calloc(1, 1228);
 b_free(ptr9);
 b_free(ptr10);
 b_free(ptr11);
 b_free(ptr12);
 b_free(ptr13);
 b_free(ptr14);
 b_free(ptr15);
 b_free(ptr16);
 b_free(ptr17);
 b_free(ptr18);
 b_free(ptr19);
 b_free(ptr20);
 b_free(ptr21);
 b_free(ptr22);
 b_free(ptr23);
 b_free(ptr24);
 b_free(ptr25);
 b_free(ptr26);
 b_free(ptr27);
 b_free(ptr28);
 b_free(ptr29);
 b_free(ptr30);
 b_free(ptr31);
 b_free(ptr32);
 b_free(ptr33);
 b_free(ptr34);
 b_free(ptr35);
 b_free(ptr36);
 b_free(ptr37);
 b_free(ptr38);
 b_free(ptr39);
 b_free(ptr40);
 b_free(ptr41);
 b_free(ptr42);
 b_free(ptr43);
 b_free(ptr44);
 b_free(ptr45);
 b_free(ptr46);
 b_free(ptr47);
 b_free(ptr48);
 b_free(ptr49);
 b_free(ptr50);
 b_free(ptr51);
 b_free(ptr52);
 b_free(ptr53);
 b_free(ptr54);
 b_free(ptr55);
 b_free(ptr56);
 b_free(ptr57);
 b_free(ptr58);
 b_free(ptr59);
 b_free(ptr60);
 b_free(ptr61);
 b_free(ptr62);
 b_free(ptr63);
 b_free(ptr64);
 b_free(ptr65);
 b_free(ptr66);
 b_free(ptr67);
 b_free(ptr68);
 b_free(ptr69);
 b_free(ptr70);
 b_free(ptr71);
 b_free(ptr72);
 b_free(ptr73);
 b_free(ptr74);
 b_free(ptr75);
 b_free(ptr76);
 b_free(ptr77);
 b_free(ptr781);
 b_free(ptr79);
 b_free(ptr80);
 b_free(ptr81);
 b_free(ptr82);
 b_free(ptr83);
 b_free(ptr84);
 b_free(ptr85);
 b_free(ptr86);
 b_free(ptr87);
 b_free(ptr88);
 b_free(ptr89);
 b_free(ptr90);
 b_free(ptr91);
 b_free(ptr92);
 b_free(ptr93);
 b_free(ptr94);
 b_free(ptr95);
 b_free(ptr96);
 b_free(ptr97);
 b_free(ptr98);
 b_free(ptr99);
 b_free(ptr100);
    return 0;

}
