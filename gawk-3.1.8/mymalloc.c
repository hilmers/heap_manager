#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#define MINIMUM_SIZE    8
#define LIST_OBJ_SIZE   sizeof(list_obj)

typedef struct list_obj list_obj;
struct list_obj {
    size_t size;
    int free;
    list_obj* next;
    char data[];
};

list_obj*   request_memory(list_obj* end, size_t size);
list_obj*   get_obj(void* ptr);
void*       malloc(size_t size);
void        free(void* ptr);
void*       calloc(size_t n_items, size_t size);
void*       realloc(void* ptr, size_t size);

static list_obj avail = { .next = NULL };

list_obj* request_memory(list_obj* end, size_t size) {
    list_obj*   block;
    block = sbrk(0);
    void* request = sbrk(size + LIST_OBJ_SIZE);
    if (request == (void*) -1)
        return NULL;
    block->size = size;
    block->next = NULL;
    block->free = 0;
    return block;
}

list_obj* get_obj(void* ptr)
{
    return (list_obj*) ((char*) ptr - LIST_OBJ_SIZE);
}

void* malloc(size_t size) {
    list_obj*    block;
    list_obj*    prev_block;
    if (size <= 0)
        return NULL; 

    if (!avail.next) {
        block = request_memory(NULL, size);
        avail.next = block;
        return block->data;
    }
    prev_block = &avail;
    block = avail.next;

    while (block && !(block->free && block->size >= size)) {
        prev_block = block;
        block = block->next;
    }

    if (!block) {
        block = request_memory(block, size);
        if (!block)
            return NULL;
        prev_block->next = block;
    } else {
        int diff = block->size - size;
        block->free = 0;
    }
    return block->data;
}

void free(void* ptr) {
    if (!ptr)
        return;

    list_obj* block_ptr = get_obj(ptr);
    block_ptr->free = 1;
}

void* calloc(size_t n, size_t n_size)
{
    size_t pref_size = n * n_size;
    void* data = malloc(pref_size);

    if (!data)
        return NULL;
    memset(data, 0, pref_size);
    return data;
}

void* realloc(void* data, size_t size) 
{
    if (!data) 
        return malloc(size);

    list_obj* old_block = get_obj(data);

    if(old_block->size >= size) 
        return data;

    void* realloced_data = malloc(size);
    if (!realloced_data)
        return NULL;

    memcpy(realloced_data, data, old_block->size);
    free(data);
    return realloced_data;
}
