#ifndef PARAM_H
#define PARAM_H

#include <mercury.h>
#include <mercury_macros.h>

MERCURY_GEN_PROC(gamma_in_t,
        ((int32_t)(n))\
        ((hg_bulk_t)(bulk))\
        ((int32_t)(compute))\
        ((int32_t)(memory))\
        ((int32_t)(file_size)))

MERCURY_GEN_PROC(gamma_out_t, ((int32_t)(ret)))

#endif
