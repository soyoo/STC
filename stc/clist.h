/* MIT License
 *
 * Copyright (c) 2020 Tyge Løvset, NORCE, www.norceresearch.no
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef CLIST__H__
#define CLIST__H__

#include <stdlib.h>
#include "cdefs.h"

/*  Circular Singly-linked Lists.

    This implements a std::forward_list-like class in C, but because it is circular,
    it also support push* and splice* at both ends of the list. This makes it ideal
    for being used as a queue, unlike std::forward_list. Basic usage is similar to cvec:

    #include <stdio.h>
    #include <stc/clist.h>
    #include <stc/crandom.h>
    declare_clist(ix, int64_t);

    int main() {
        clist_ix list = clist_ini;
        crand_rng32_t pcg = crand_rng32_init(12345);
        int n;
        for (int i=0; i<1000000; ++i) // one million
            clist_ix_push_back(&list, crand_i32(&pcg));
        n = 0;
        c_foreach (i, clist_ix, list)
            if (++n % 10000 == 0) printf("%8d: %10zd\n", n, i.item->value);
        // Sort them...
        clist_ix_sort(&list); // mergesort O(n*log n)
        n = 0;
        puts("sorted");
        c_foreach (i, clist_ix, list)
            if (++n % 10000 == 0) printf("%8d: %10zd\n", n, i.item->value);
        clist_ix_destroy(&list);
    }
*/

#define declare_clist(...)   c_MACRO_OVERLOAD(declare_clist, __VA_ARGS__)

#define declare_clist_2(X, Value) \
                             declare_clist_3(X, Value, c_default_destroy)
#define declare_clist_3(X, Value, valueDestroy) \
                             declare_clist_4(X, Value, valueDestroy, c_default_compare)
#define declare_clist_4(X, Value, valueDestroy, valueCompare) \
                             declare_clist_7(X, Value, valueDestroy, Value, \
                                             valueCompare, c_default_to_raw, c_default_from_raw)
#define declare_clist_str()  declare_clist_7(str, cstr_t, cstr_destroy, const char*, \
                                             cstr_compare_raw, cstr_to_raw, cstr_make)

#define declare_clist_types(X, Value) \
    typedef struct clist_##X##_node { \
        struct clist_##X##_node* next; \
        Value value; \
    } clist_##X##_node_t; \
\
    typedef struct clist_##X { \
        clist_##X##_node_t* last; \
    } clist_##X; \
\
    typedef struct { \
        clist_##X##_node_t* const* _last; \
        clist_##X##_node_t* item; \
        int state; \
    } clist_##X##_iter_t

#define clist_ini           {NULL}
#define clist_empty(list)   ((list).last == NULL)

#define c_emplace_after(self, ctype, pos, items) do { \
    ctype* __self = self; \
    ctype##_iter_t __pos = pos; \
    const ctype##_input_t __arr[] = {items}; \
    for (size_t __i=0; __i<sizeof(__arr)/sizeof(__arr[0]); ++__i) \
        __pos = ctype##_emplace_after(__self, __pos, __arr[__i]); \
} while (0)

declare_clist_types(void, int);
STC_API size_t _clist_size(const clist_void* self);

#define declare_clist_7(X, Value, valueDestroy, RawValue, valueCompareRaw, valueToRaw, valueFromRaw) \
\
    declare_clist_types(X, Value); \
    typedef Value clist_##X##_value_t; \
    typedef RawValue clist_##X##_rawvalue_t; \
    typedef clist_##X##_rawvalue_t clist_##X##_input_t; \
\
    STC_INLINE clist_##X \
    clist_##X##_init(void) {clist_##X x = clist_ini; return x;} \
    STC_INLINE bool \
    clist_##X##_empty(clist_##X ls) {return clist_empty(ls);} \
    STC_INLINE size_t \
    clist_##X##_size(clist_##X ls) {return _clist_size((const clist_void*) &ls);} \
    STC_INLINE Value \
    clist_##X##_value_from_raw(RawValue rawValue) {return valueFromRaw(rawValue);} \
\
    STC_API void \
    clist_##X##_destroy(clist_##X* self); \
    STC_INLINE void \
    clist_##X##_clear(clist_##X* self) {clist_##X##_destroy(self);} \
\
    STC_API void \
    clist_##X##_push_n(clist_##X *self, const clist_##X##_input_t in[], size_t size); \
    STC_API void \
    clist_##X##_push_back(clist_##X* self, Value value); \
    STC_INLINE void \
    clist_##X##_emplace_back(clist_##X* self, RawValue rawValue) { \
        clist_##X##_push_back(self, valueFromRaw(rawValue)); \
    } \
    STC_API void \
    clist_##X##_push_front(clist_##X* self, Value value); \
    STC_INLINE void \
    clist_##X##_emplace_front(clist_##X* self, RawValue rawValue) { \
        clist_##X##_push_front(self, valueFromRaw(rawValue)); \
    } \
    STC_API void \
    clist_##X##_pop_front(clist_##X* self); \
\
    STC_INLINE clist_##X##_iter_t \
    clist_##X##_before_begin(const clist_##X* self) { \
        clist_##X##_iter_t it = {&self->last, self->last, -1}; return it; \
    } \
    STC_INLINE clist_##X##_iter_t \
    clist_##X##_begin(const clist_##X* self) { \
        clist_##X##_node_t* head = self->last ? self->last->next : NULL; \
        clist_##X##_iter_t it = {&self->last, head}; return it; \
    } \
    STC_INLINE clist_##X##_iter_t \
    clist_##X##_end(const clist_##X* self) { \
        clist_##X##_iter_t it = {NULL}; return it; \
    } \
    STC_INLINE void \
    clist_##X##_next(clist_##X##_iter_t* it) { \
        it->item = ((it->state += it->item == *it->_last) == 1) ? NULL : it->item->next; \
    } \
    STC_INLINE clist_##X##_value_t* \
    clist_##X##_itval(clist_##X##_iter_t it) {return &it.item->value;} \
\
    STC_API clist_##X##_iter_t \
    clist_##X##_insert_after(clist_##X* self, clist_##X##_iter_t pos, Value value); \
    STC_INLINE clist_##X##_iter_t \
    clist_##X##_emplace_after(clist_##X* self, clist_##X##_iter_t pos, RawValue rawValue) { \
        return clist_##X##_insert_after(self, pos, valueFromRaw(rawValue)); \
    } \
    STC_API clist_##X##_iter_t \
    clist_##X##_erase_after(clist_##X* self, clist_##X##_iter_t pos); \
    STC_INLINE clist_##X##_iter_t \
    clist_##X##_erase_range_after(clist_##X* self, clist_##X##_iter_t pos, clist_##X##_iter_t last) { \
        while (pos.item != last.item) pos = clist_##X##_erase_after(self, pos); \
        return pos; \
    } \
\
    STC_INLINE void \
    clist_##X##_splice_after(clist_##X* self, clist_##X##_iter_t pos, clist_##X* other) { \
        _clist_splice_after((clist_void *) self, *(clist_void_iter_t *) &pos, (clist_void *) other); \
    } \
    STC_INLINE void \
    clist_##X##_splice_front(clist_##X* self, clist_##X* other) { \
        clist_##X##_splice_after(self, clist_##X##_before_begin(self), other); \
    } \
    STC_INLINE void \
    clist_##X##_splice_back(clist_##X* self, clist_##X* other) { \
        clist_##X##_iter_t last = {&self->last, self->last, 0}; \
        clist_##X##_splice_after(self, last, other); \
    } \
\
    STC_API clist_##X##_iter_t \
    clist_##X##_find_before(const clist_##X* self, clist_##X##_iter_t first, clist_##X##_iter_t last, RawValue val); \
    STC_API clist_##X##_iter_t \
    clist_##X##_find(const clist_##X* self, RawValue val); \
    STC_API size_t \
    clist_##X##_remove(clist_##X* self, RawValue val); \
    STC_API void \
    clist_##X##_sort(clist_##X* self); \
\
    STC_INLINE Value* \
    clist_##X##_front(clist_##X* self) {return &self->last->next->value;} \
    STC_INLINE Value* \
    clist_##X##_back(clist_##X* self) {return &self->last->value;} \
\
    implement_clist_7(X, Value, valueDestroy, RawValue, valueCompareRaw, valueToRaw, valueFromRaw)


/* -------------------------- IMPLEMENTATION ------------------------- */

#if !defined(STC_HEADER) || defined(STC_IMPLEMENTATION)
#define implement_clist_7(X, Value, valueDestroy, RawValue, valueCompareRaw, valueToRaw, valueFromRaw) \
\
    STC_API void \
    clist_##X##_destroy(clist_##X* self) { \
        while (self->last) \
            clist_##X##_pop_front(self); \
    } \
\
    STC_API void \
    clist_##X##_push_back(clist_##X* self, Value value) { \
        _clist_insert_after(self, X, self->last, value); \
        self->last = entry; \
    } \
    STC_API void \
    clist_##X##_push_front(clist_##X* self, Value value) { \
        _clist_insert_after(self, X, self->last, value); \
        if (!self->last) self->last = entry; \
    } \
    STC_API void \
    clist_##X##_push_n(clist_##X *self, const clist_##X##_input_t in[], size_t size) { \
        for (size_t i=0; i<size; ++i) clist_##X##_push_back(self, valueFromRaw(in[i])); \
    } \
    STC_API void \
    clist_##X##_pop_front(clist_##X* self) { \
        _clist_erase_after(self, X, self->last, valueDestroy); \
    } \
\
    STC_API clist_##X##_iter_t \
    clist_##X##_insert_after(clist_##X* self, clist_##X##_iter_t pos, Value value) { \
        _clist_insert_after(self, X, pos.item, value); \
        if (pos.item == self->last && pos.state == 0) self->last = entry; \
        pos.item = entry; return pos; \
    } \
    STC_API clist_##X##_iter_t \
    clist_##X##_erase_after(clist_##X* self, clist_##X##_iter_t pos) { \
        _clist_erase_after(self, X, pos.item, valueDestroy); \
        clist_##X##_next(&pos); return pos; \
    } \
\
    STC_API clist_##X##_iter_t \
    clist_##X##_find_before(const clist_##X* self, clist_##X##_iter_t first, clist_##X##_iter_t last, RawValue val) { \
        clist_##X##_iter_t i = first; \
        for (clist_##X##_next(&i); i.item != last.item; clist_##X##_next(&i)) { \
            RawValue r = valueToRaw(&i.item->value); \
            if (valueCompareRaw(&r, &val) == 0) return first; \
            first = i; \
        } \
        return clist_##X##_end(self); \
    } \
\
    STC_API clist_##X##_iter_t \
    clist_##X##_find(const clist_##X* self, RawValue val) { \
        clist_##X##_iter_t it = clist_##X##_find_before(self, clist_##X##_before_begin(self), clist_##X##_end(self), val); \
        if (it.item != clist_##X##_end(self).item) clist_##X##_next(&it); \
        return it; \
    } \
\
    STC_API size_t \
    clist_##X##_remove(clist_##X* self, RawValue val) { \
        size_t n = 0; \
        clist_##X##_iter_t it = clist_##X##_before_begin(self), end = clist_##X##_end(self); \
        while ((it = clist_##X##_find_before(self, it, clist_##X##_end(self), val)).item != clist_##X##_end(self).item) \
            clist_##X##_erase_after(self, it), ++n; \
        return n; \
    } \
\
    static inline int \
    clist_##X##_sort_compare(const void* x, const void* y) { \
        RawValue a = valueToRaw(&((clist_##X##_node_t *) x)->value); \
        RawValue b = valueToRaw(&((clist_##X##_node_t *) y)->value); \
        return valueCompareRaw(&a, &b); \
    } \
    STC_API void \
    clist_##X##_sort(clist_##X* self) { \
        clist_void_node_t* last = _clist_mergesort((clist_void_node_t *) self->last->next, clist_##X##_sort_compare); \
        self->last = (clist_##X##_node_t *) last; \
    } \
    typedef int clist_##X##_dud


#define _clist_insert_after(self, X, node, val) \
    clist_##X##_node_t *entry = c_new (clist_##X##_node_t), \
                         *next = self->last ? node->next : entry; \
    entry->value = val; \
    entry->next = next; \
    if (node) node->next = entry
    /* +: set self->last based on node */

#define _clist_erase_after(self, X, node, valueDestroy) \
    clist_##X##_node_t* del = node->next, *next = del->next; \
    node->next = next; \
    if (del == next) self->last = NULL; \
    else if (self->last == del) self->last = node; \
    valueDestroy(&del->value); \
    free(del)


STC_API void
_clist_splice_after(clist_void* self, clist_void_iter_t pos, clist_void* other) {
    if (!pos.item)
        self->last = other->last;
    else if (other->last) {
        clist_void_node_t *next = pos.item->next;
        pos.item->next = other->last->next;
        other->last->next = next;
        if (pos.item == self->last && pos.state == 0) self->last = other->last;
    }
    other->last = NULL;
}

STC_API size_t
_clist_size(const clist_void* self) {
    const clist_void_node_t *i = self->last;
    if (!i) return 0;
    size_t n = 1;
    while ((i = i->next) != self->last) ++n;
    return n;
}

/* Singly linked list Mergesort implementation by Simon Tatham. O(n*log n).
 * https://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
 */
STC_API clist_void_node_t *
_clist_mergesort(clist_void_node_t *list, int (*cmp)(const void*, const void*)) {
    clist_void_node_t *p, *q, *e, *tail, *oldhead;
    int insize = 1, nmerges, psize, qsize, i;
    if (!list) return NULL;

    while (1) {
        p = list;
        oldhead = list;
        list = tail = NULL;
        nmerges = 0;

        while (p) {
            ++nmerges;
            q = p;
            psize = 0;
            for (i = 0; i < insize; ++i) {
                ++psize;
                q = (q->next == oldhead ? NULL : q->next);
                if (!q) break;
            }
            qsize = insize;

            while (psize > 0 || (qsize > 0 && q)) {
                if (psize == 0) {
                    e = q; q = q->next; --qsize;
                    if (q == oldhead) q = NULL;
                } else if (qsize == 0 || !q) {
                    e = p; p = p->next; --psize;
                    if (p == oldhead) p = NULL;
                } else if (cmp(p, q) <= 0) {
                    e = p; p = p->next; --psize;
                    if (p == oldhead) p = NULL;
                } else {
                    e = q; q = q->next; --qsize;
                    if (q == oldhead) q = NULL;
                }
                if (tail)
                    tail->next = e;
                else
                    list = e;
                tail = e;
            }
            p = q;
        }
        tail->next = list;

        if (nmerges <= 1)
            return tail;

        insize *= 2;
    }
}

#else
#define implement_clist_7(X, Value, valueDestroy, RawValue, valueCompareRaw, valueToRaw, valueFromRaw)
#endif

#endif
