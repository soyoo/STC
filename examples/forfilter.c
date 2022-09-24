#include <stdio.h>
#define i_implement
#include <stc/cstr.h>
#include <stc/csview.h>

#define i_type IVec
#define i_val int
#include <stc/cstack.h>

#define i_type SVec
#define i_val_bind csview
#include <stc/cstack.h>

// filters and transforms:
#define flt_remove(i, x) (*i.ref != (x))
#define flt_even(i) ((*i.ref & 1) == 0)
#define trf_square(i) (*i.ref * *i.ref)

void demo1(void)
{
    c_auto (IVec, vec) {
        c_forlist (i, int, {0, 1, 2, 3, 4, 5, 80, 6, 7, 80, 8, 9, 80, 10, 11, 12, 13, 14, 15, 80, 16, 17})
            IVec_push(&vec, *i.ref);

        c_forfilter (i, IVec, vec, flt_remove(i, 80))
            printf(" %d", *i.ref);
        puts("");

        int res, sum = 0;
        c_forfilter (i, IVec, vec, c_flt_drop(i, 3)
                                &&   flt_even(i)
                                &&   flt_remove(i, 80)
                              //&& c_flt_take(i, 5)
                                 , c_flt_take(i, 5)
        ){
            sum += res = trf_square(i);
            printf(" %d:%d", *i.ref, res);
        }
        printf("\nsum: %d\n", sum);
    }
}


/* Rust:
fn main() {
    let vector = (1..)            // Infinite range of integers
        .filter(|x| x % 2 != 0)   // Collect odd numbers
        .take(5)                  // Only take five numbers
        .map(|x| x * x)           // Square each number
        .collect::<Vec<usize>>(); // Return as a new Vec<usize>
    println!("{:?}", vector);     // Print result
}
*/

void demo2(void)
{
    c_auto (IVec, vector) {
        int n = 0;
        for (size_t x=1;; ++x)
            if (n == 5) break;
            else if (x & 1) ++n, IVec_push(&vector, x*x);

        c_foreach (i, IVec, vector) printf(" %d", *i.ref);
        puts("");
    }
}

/* Rust:
fn main() {
    let sentence = "This is a sentence in Rust.";
    let words: Vec<&str> = sentence
        .split_whitespace()
        .collect();
    let words_containing_i: Vec<&str> = words
        .into_iter()
        .filter(|word| word.contains("i"))
        .collect();
    println!("{:?}", words_containing_i);
}
*/
void demo3(void)
{
    c_auto (SVec, words, words_containing_i) {
        const char* sentence = "This is a sentence in C99.";
        c_fortoken (w, sentence, " ")
            SVec_push(&words, *w.ref);

        c_forfilter (w, SVec, words, 
                     csview_contains(*w.ref, c_sv("i")))
            SVec_push(&words_containing_i, *w.ref);

        c_foreach (w, SVec, words_containing_i)
            printf(" %.*s", c_ARGsv(*w.ref));
        puts("");
    }
}

void demo4(void)
{
    csview s = c_sv("ab123cReAghNGnΩoEp");
    cstr out = cstr_null;

    c_forfilter (i, csview, s, utf8_isupper(utf8_peek(i.ref))) {
        char buf[4];
        utf8_encode(buf, utf8_tolower(utf8_peek(i.ref)));
        cstr_push(&out, buf);
    }

    printf("%s\n", cstr_str(&out));
}


int main(void)
{
    demo1();
    demo2();
    demo3();
    demo4();
}

#include "../src/utf8code.c"