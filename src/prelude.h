#ifndef __PRELUDE_H__
#define __PRELUDE_H__

#include <stdint.h>
#include <stdio.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

typedef float  f32;
typedef double f64;

typedef FILE File;

typedef enum {
    FALSE = 0,
    TRUE = 1,
} Bool;

#define ERROR(x)                     \
    {                                \
        fprintf(stderr,              \
                "%s:%s:%d \"%s\"\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                x);                  \
        exit(EXIT_FAILURE);          \
    }

#endif
