#ifndef SERVICE_MACROS_H
#define SERVICE_MACROS_H

/*
  * (C) 2015 The University of Chicago
  *
  * See COPYRIGHT in top-level directory.
*/

#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <mpi.h>
#include "network-server.h"
#include "compute-server.h"
#include "storage-server.h"
#include "memory-server.h"

static unsigned int provider_id_counter = 0;

#define GENERATE_UNIQUE_PROVIDER_ID() \
    provider_id_counter++;
    
#define INIT_MARGO(connection_type, num_threads) \
    MPI_Init(&argc, &argv);\
    int rank;\
    int size;\
    char filename[100];\
    char addr_str[128];\
    char *temp;\
    char actual_addr[128];\
    size_t addr_str_size = 128;\
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);\
    MPI_Comm_size(MPI_COMM_WORLD, &size);\
    sprintf(filename, "server_addr_%d.txt", rank);\
    FILE *fp = fopen(filename, "w");\
    margo_instance_id mid = margo_init(#connection_type, MARGO_SERVER_MODE, 0, num_threads);\
    assert(mid);\
    hg_addr_t my_address;\
    margo_addr_self(mid, &my_address);\
    margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);\
    temp = addr_str + 17;\
    sprintf(actual_addr, "%s%s", #connection_type, temp);\
    fprintf(fp, "%s\n", actual_addr);\
    fflush(fp);\
    fclose(fp);\
    MPI_Barrier(MPI_COMM_WORLD);

#define FINALIZE_MARGO(enable_remote_shutdown) \
    margo_addr_free(mid, my_address);\
    if(enable_remote_shutdown) \
      margo_enable_remote_shutdown(mid);\
    margo_wait_for_finalize(mid);\
    MPI_Finalize();

#define INIT_AND_RUN_SERVICE(name, d) \
    initialize_##name##_service(mid, d); \
    MPI_Barrier(MPI_COMM_WORLD); \
    name##_write_local_provider_ids(rank, d); \
    MPI_Barrier(MPI_COMM_WORLD); \
    name##_initialize_remote_provider_handles(mid, rank, size);

#define FINALIZE_SERVICE(name, d) \
    finalize_##name##_service(mid, d)

#endif
