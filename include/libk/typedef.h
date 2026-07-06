#ifndef OUROBOROS_LIBK_TYPEDEF_H
#define OUROBOROS_LIBK_TYPEDEF_H

// zero means success, everything else means failiure
typedef int err_t;
#define IS_SUCCESS(err) ((err)==0)
#define SUCCESS ((err_t)0)
#define GENERIC_FAIL ((err_t)-1)

#endif
