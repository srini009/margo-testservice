#ifndef CLIENT_HELPER_H
#define CLIENT_HELPER_H

/*
  * (C) 2015 The University of Chicago
  *
  * See COPYRIGHT in top-level directory.
*/

#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <mpi.h>
#include <unistd.h>

#include "defaults.h"
#include "network-client.h"
#include "compute-client.h"
#include "storage-client.h"
#include "memory-client.h"


#define INIT_MARGO(connection_type, num_threads) \
    if(argc != 2) {\
        fprintf(stderr,"Usage: %s <number of server processes>\n", argv[0]);\
        exit(0);\
    }\
    MPI_Init(&argc, &argv);\
    int rank, comm_size;\
    int num_servers = atoi(argv[1]);\
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);\
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);\
    margo_instance_id mid = margo_init(#connection_type, MARGO_CLIENT_MODE, 0, num_threads);\
    MPI_Barrier(MPI_COMM_WORLD);

#define INIT_CLIENT(name) \
    initialize_##name##_client(mid, num_servers); 

#define FINALIZE_CLIENT(name) \
    finalize_##name##_client(mid, num_servers); 

#define FINALIZE_MARGO(enable_remote_shutdown) \
    if(!rank) {\
      for(int r = 0; r < num_servers; r++) {\
        char filename[100];\
	char svr_addr_str[128];\
        sprintf(filename, "server_addr_%d.txt", r);\
        FILE * fp;\
        fp = fopen(filename, "r");\
        fscanf(fp, "%s", svr_addr_str);\
        hg_addr_t svr_addr;\
        margo_addr_lookup(mid, svr_addr_str, &svr_addr);\
        margo_shutdown_remote_instance(mid, svr_addr);\
        fclose(fp);\
      }\
    }\
    margo_finalize(mid);\
    MPI_Finalize();

#endif
