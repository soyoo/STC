#include <stc/crandom.h>
#include <stc/cqueue.h>
#include <stdio.h>

declare_clist(i, int);
declare_cqueue(i, clist_i); // min-heap (increasing values)

int main() {
    int n = 10000000;
    crand_rng32_t gen = crand_rng32_init(1234);
    crand_uniform_i32_t dist = crand_uniform_i32_init(gen, 0, n);

    cqueue_i queue = cqueue_i_init();

    // Push ten million random numbers onto the queue.
    for (int i=0; i<n; ++i)
        cqueue_i_push(&queue, crand_uniform_i32(&dist));
    
    // Push or pop on the queue ten million times
    printf("%d\n", n);
    for (int i=n; i>0; --i) {
        int r = crand_uniform_i32(&dist);
        if (r & 1)
            ++n, cqueue_i_push(&queue, r);
        else
            --n, cqueue_i_pop(&queue);
    }
    printf("%d\n", n);
    printf("%zu\n", n, cqueue_i_size(queue));
    cqueue_i_destroy(&queue);
}