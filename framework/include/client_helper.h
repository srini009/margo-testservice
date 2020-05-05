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
#include "jsmn.h"

#include <user_clients.h>

struct Workload {
    hg_string_t *request_structure_array;
    int *op;
    int *service_id;
    int *rate_array;
    AccessPattern accessPattern;
    int *workload_factor;
    int N;
};

typedef struct Workload Workload;

void substring(char s[], char sub[], int p, int l) {
   int c = 0;
   
   while (c < l) {
      sub[c] = s[p+c-1];
      c++;
   }
   sub[c] = '\0';
}

int num_children(jsmntok_t *t, int num, int *index) {

  *index = -1;
  int n = 0;
  for(int i = 0; i < num; i++) {
    if(t[i].type == JSMN_ARRAY) {
      n = t[i].size;
      *index = i;
      break;  
    }
  } 

  return n;
}

void extract_first_link_info(jsmntok_t *t, char *request, int *microservice_id, int *service_id) {
  char fname[50], sname[50];
  assert(t[2].type == JSMN_PRIMITIVE);
  substring(request, fname, t[2].start+1, 1);
  *service_id = atoi(sname);

  assert(t[4].type == JSMN_PRIMITIVE);
  substring(request, fname, t[4].start+1, 1);
  *microservice_id = atoi(fname);
}
 
#define INIT_MARGO(connection_type, num_threads) \
    if(argc != 2) {\
        fprintf(stderr,"Usage: %s <number of server processes>\n", argv[0]);\
        exit(0);\
    }\
    MPI_Init(&argc, &argv);\
    int rank, comm_size;\
    int result;\
    int num_servers = atoi(argv[1]);\
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);\
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);\
    margo_instance_id mid = margo_init(#connection_type, MARGO_CLIENT_MODE, 0, num_threads);\
    MPI_Barrier(MPI_COMM_WORLD);\
    jsmn_parser p; \
    jsmn_init(&p); \
    int my_assigned_server = rand()%num_servers;

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
   w.op = (int*) malloc(sizeof(int)*N);\
   w.service_id = (int*) malloc(sizeof(int)*N);\
   w.N = N;\
   for(int i=0;i<N; i++) {\
     jsmntok_t t[128]; /* We expect no more than 128 JSON tokens */\
     w.rate_array[i] = rate[i];\
     w.workload_factor[i] = workload_factor[i];\
     w.request_structure_array[i] = get_##service_name##_service_request_structure(op_array[i]);\
     jsmn_parse(&p, w.request_structure_array[i], strlen(w.request_structure_array[i]), t, 128); \
     extract_first_link_info(t, w.request_structure_array[i], &w.op[i], &w.service_id[i]);\
   }

#define RUN_WORKLOAD(service_name, w) \
   switch(w.accessPattern) {\
     case(Fixed): \
       for(int i = 0; i < w.N; i++) {\
         generate_request(w.service_id[i], w.op[i], w.accessPattern, w.workload_factor[i], NULL, w.request_structure_array[i], my_assigned_server, &result);\
         usleep(w.rate_array[i]);\
       }\
       break;\
     case(Dynamic): \
       for(int i = 0; i < w.N; i++) {\
         generate_request(w.service_id[i], w.op[i], w.accessPattern, w.workload_factor[i], NULL, w.request_structure_array[i], num_servers, &result);\
         usleep(w.rate_array[i]);\
       }\
       break;\
   }\
   MPI_Barrier(MPI_COMM_WORLD);

#define CLEANUP_WORKLOAD(w) \
   free(w.request_structure_array);\
   free(w.rate_array);\
   free(w.workload_factor);\
   free(w.op);\
   free(w.service_id);

#endif
