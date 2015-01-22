#define __FISH_xsH_H 1

#include <assert.h> // multiple ok.

// or intern?? 
#include "EXTERN.h"
#include <perl.h>
// hv_iter etc.
#include <XSUB.h>

#define xsH_hv_iter_while hv_iter(config, xsH_hv_iter_args_refs)

#define xsH_hv_iter_args_init char* __key; int __val_i; double __val_d; char* __val_s; int __type;

#define xsH_hv_iter_args_refs &__key, &__type, &__val_i, &__val_d, &__val_s

#define xsH_hv_iter_init \
    assert (hv_iterinit(config)); \
    xsH_hv_iter_args_init;

#define xsH_hv_iter_getint(str_wanted, store_var) \
    do { \
        if (! strcmp(__key, str_wanted)) {  \
            if (__type == 1) \
                store_var = __val_i; \
            else if (__type == 2) \
                store_var = (int) __val_d; \
            else if (__type == 3) { \
                errno = 0; \
                char* endptr; \
                store_var = (int) strtod(__val_s, &endptr); \
                if (errno) { \
                    fprintf(stderr, "Can't convert string (%s) to int: (errno %d)\n", __val_s, errno); \
                } \
                else if (endptr == __val_s) { \
                    fprintf(stderr, "Can't convert string (%s) to int.\n", __val_s); \
                } \
            } \
        } \
    } while(0) ;

#define xsH_hv_iter_getdouble(str_wanted, store_var) \
    do { \
        if (! strcmp(__key, str_wanted)) {  \
            if (__type == 1) { \
                store_var = (double) __val_i; \
            } \
            else if (__type == 2) { \
                store_var = __val_d; \
            } \
            else if (__type == 3) { \
                errno = 0; \
                char* endptr; \
                store_var = strtod(__val_s, &endptr); \
                if (errno) { \
                    fprintf(stderr, "Can't convert string (%s) to double: (errno %d)\n", __val_s, errno); \
                } \
                else if (endptr == __val_s) { \
                    fprintf(stderr, "Can't convert string (%s) to double.\n", __val_s); \
                } \
                /*if (errno) warn(perr());*/ \
            } \
        } \
    } while(0) ;


