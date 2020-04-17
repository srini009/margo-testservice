#ifndef PARAM_H
#define PARAM_H

#include <mercury.h>
#include <mercury_macros.h>
#include <mercury_proc_string.h>

MERCURY_GEN_PROC(symbio_in_t,
        ((int32_t)(workload_factor))\
        ((hg_bulk_t)(bulk))\
        ((hg_string_t)(request_structure)))

MERCURY_GEN_PROC(symbio_out_t, ((int32_t)(ret)))

#endif
