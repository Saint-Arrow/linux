#include "c-smart-pointers.h"
struct meta {
    void (*dtor)(void *);
    void *ptr;
};

static struct meta *get_meta(void *ptr) {
    return ptr - sizeof (struct meta);
}

//__attribute__((malloc)) 
void *smalloc(size_t size, void (*dtor)(void *)) {
    struct meta *meta = malloc(sizeof (struct meta) + size);
    *meta = (struct meta) {
        .dtor = dtor,
        .ptr  = meta + 1
    };
    return meta->ptr;
}

void sfree(void *ptr) {
    if (ptr == NULL)
        return;
    struct meta *meta = get_meta(ptr);
    assert(ptr == meta->ptr); // ptr shall be a pointer returned by smalloc
    meta->dtor(ptr);
    free(meta);
}


int free_no_action(void *ptr)
{
}
int free_fclose(void *ptr)
{
    fclose(*(FILE **)ptr);
}
