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


struct Workload {
    hg_string_t *request_structure_array;
    int *rate_array;
    AccessPattern accessPattern;
    int *workload_factor;
    int N;
};

typedef struct Workload Workload;
 
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

#define GENERATE_WORKLOAD(service_name, op_array, workload_factor, rate, N, accessPattern, w) \
   w.request_structure_array = (hg_string_t*)malloc(sizeof(hg_string_t)*N);\
   w.rate_array = (int*) malloc(sizeof(int)*N);\
   w.accessPattern = accessPattern;\
   w.workload_factor = (int*) malloc(sizeof(int)*N);\
   w.N = N;\
   for(int i=0;i<N; i++) {\
     w.rate_array[i] = rate[i];\
     w.workload_factor[i] = workload_factor[i];\
     w.request_structure_array[i] = get_##service_name##_service_request_structure(op_array[i]);\
   }

#endif
