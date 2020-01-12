#ifndef PARAM_H
#define PARAM_H

#include <mercury.h>
#include <mercury_macros.h>

MERCURY_GEN_PROC(alpha_in_t,
        ((int32_t)(n))\
        ((hg_bulk_t)(bulk)))


MERCURY_GEN_PROC(alpha_out_t, ((int32_t)(ret)))

#endif
