/* buddy.c
 *
 * Description:
 *   Implement buddy system for memory management
 *
 * Idea & part of the code are from https://github.com/wuwenbin/buddy2 
 */

// #include <stdio.h>
// #include <assert.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdbool.h>

// #include "types.h"
#include "buddy_system.h"

static inline int left_child(int index)
{
    /* index * 2 + 1 */
    return ((index << 1) + 1); 
}

static inline int right_child(int index)
{
    /* index * 2 + 2 */
    return ((index << 1) + 2);
}

static inline int parent(int index)
{
    /* (index+1)/2 - 1 */
    return (((index+1)>>1) - 1);
}

static inline int is_power_of_2(int index)
{
    return !(index & (index - 1));
}

#define max(a, b) (((a)>(b))?(a):(b))
#define min(a, b) (((a)<(b))?(a):(b))

/* a wrapper for free */
static void b_free(void *addr)
{
    // free(addr);
}

static inline unsigned next_power_of_2(unsigned size)
{
    /* depend on the fact that size < 2^32 */
    size -= 1;
    size |= (size >> 1);
    size |= (size >> 2);
    size |= (size >> 4);
    size |= (size >> 8);
    size |= (size >> 16);
    return size + 1;
}

/** allocate a new buddy structure 
 * @param num_of_fragments number of fragments of the memory to be managed 
 * @return pointer to the allocated buddy structure */
struct buddy *buddy_new(unsigned num_of_fragments, uint32_t heap_start)
{
    struct buddy *self = 0;
    uint32_t node_size;

    int i;

    if (num_of_fragments < 1 || !is_power_of_2(num_of_fragments)) {
        return 0;
    }

    /* alloacte an array to represent a complete binary tree */
    self = (struct buddy *) heap_start;
    // uint32_t heap_end = heap_start + sizeof(struct buddy) + 
    //                     2 * num_of_fragments * sizeof(uint32_t);

    self->size = num_of_fragments;
    node_size = num_of_fragments * 2;
    
    /* initialize *longest* array for buddy structure */
    int iter_end = num_of_fragments * 2 - 1;
    for (i = 0; i < iter_end; i++) {
        if (is_power_of_2(i+1)) {
            node_size >>= 1;
        }
        self->longest[i] = node_size;
    }

    return self;
}

void buddy_destory(struct buddy *self)
{
    b_free(self);
}

/* choose the child with smaller longest value which is still larger
 * than *size* */
unsigned choose_better_child(struct buddy *self, unsigned index, uint32_t size)
{
    struct compound {
        uint32_t size;
        unsigned index;
    } children[2];
    children[0].index = left_child(index);
    children[0].size = self->longest[children[0].index];
    children[1].index = right_child(index);
    children[1].size = self->longest[children[1].index];

    int min_idx = (children[0].size <= children[1].size) ? 0: 1;

    if (size > children[min_idx].size) {
        min_idx = 1 - min_idx;
    }
    
    return children[min_idx].index;
}

/** allocate *size* from a buddy system *self* 
 * @return the offset from the beginning of memory to be managed */
int buddy_alloc(struct buddy *self, uint32_t size)
{
    if (self == 0 || self->size < size) {
        return -1;
    }
    size = next_power_of_2(size);

    unsigned index = 0;
    if (self->longest[index] < size) {
        return -1;
    }

    /* search recursively for the child */
    unsigned node_size = 0;
    for (node_size = self->size; node_size != size; node_size >>= 1) {
        /* choose the child with smaller longest value which is still larger
         * than *size* */
        /* TODO */
        index = choose_better_child(self, index, size);
    }

    /* update the *longest* value back */
    self->longest[index] = 0;
    int offset = (index + 1)*node_size - self->size;

    while (index) {
        index = parent(index);
        self->longest[index] = 
            max(self->longest[left_child(index)],
                self->longest[right_child(index)]);
    }

    return offset;
}

void buddy_free(struct buddy *self, int offset)
{
    if (self == 0 || offset < 0 || offset > self->size) {
        return;
    }

    uint32_t node_size;
    unsigned index;

    /* get the corresponding index from offset */
    node_size = 1;
    index = offset + self->size - 1;

    for (; self->longest[index] != 0; index = parent(index)) {
        node_size <<= 1;    /* node_size *= 2; */

        if (index == 0) {
            break;
        }
    }

    self->longest[index] = node_size;

    while (index) {
        index = parent(index);
        node_size <<= 1;

        uint32_t left_longest = self->longest[left_child(index)];
        uint32_t right_longest = self->longest[right_child(index)];

        if (left_longest + right_longest == node_size) {
            self->longest[index] = node_size;
        } else {
            self->longest[index] = max(left_longest, right_longest);
        }
    }
}

void buddy_dump(struct buddy *self)
{   
    printf("*** Page Map ***");

    int len = self->size << 1;
    int max_col = self->size << 1; 
    int level = 0;
    int i,j;

    char cs[] = {'/', '\\'};
    int idx = 0;
    char c;

    for (i = 0, max_col=len, level=0; i < len-1; i++) {
        if (is_power_of_2(i+1)) {
            max_col >>= 1;
            level ++;
            idx = 0;
            printf("\n\t%d(%.2dK): ", level, max_col);
        }

        for (int k = 0; k < max_col - 1; k++)
            printf(" ");
        printf("%ld", self->longest[i]);
    }

    for (i = 0, max_col=len, level=0; i < len-1; i++) {
        if (is_power_of_2(i+1)) {
            max_col >>= 1;
            level ++;
            idx = 0;
            printf("\n\t%d(%.2dK): ", level, max_col);
        }

        if (self->longest[i] > 0) {
            c = '-';
        } else {
            c = cs[idx];
            idx ^= 0x1;
        }

        for (j = 0; j < max_col; j++) {
            printf("%c", c);
        }
    }
    printf("\n\n");
}

int buddy_size(struct buddy *self, int offset)
{
    unsigned node_size = 1;
    unsigned index = offset + self->size - 1;

    for (; self->longest[index]; index = parent(index)) {
        node_size >>= 1;
    }

    return node_size;
}
