#ifndef TYPES_H
#define TYPES_H

#include <mercury.h>
#include <mercury_macros.h>
#include <mercury_proc_string.h>

MERCURY_GEN_PROC(symbio_in_t,
        ((int32_t)(workload_factor))\
        ((hg_bulk_t)(bulk))\
        ((hg_string_t)(request_structure)))

MERCURY_GEN_PROC(symbio_out_t, ((int32_t)(ret)))

enum AccessPattern {Fixed = 0, Dynamic = 1};

enum Microservices {Network = 0, Memory = 1, Compute = 2, Storage = 3};

typedef enum AccessPattern AccessPattern;
typedef enum Microservices Microservices;

#endif
