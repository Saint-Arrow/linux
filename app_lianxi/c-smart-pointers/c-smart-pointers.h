#ifndef smart_point
#define smart_point

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#define smart __attribute__((cleanup(sfree_stack)))

void *smalloc(size_t size, void (*dtor)(void *));
void sfree(void *ptr);

__attribute__ ((always_inline))
inline void sfree_stack(void *ptr) {
    sfree(*(void **) ptr);
}

int free_no_action(void *ptr);
int free_fclose(void *ptr);
#endif
