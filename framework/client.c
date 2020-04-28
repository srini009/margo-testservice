#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include "alpha-client.h"
#include "common.h"
#include <mpi.h>
#include <unistd.h>


void generate_request_characteristics(int32_t * transfer_size, int32_t * compute, int32_t * memory, int32_t * file_size, int32_t * num_requests, int32_t * sleeptime)
{
    /* These can be dynamically modified! */
    *transfer_size = TRANSFER_SIZE;
    *compute = COMPUTE_CYCLES;
    *memory = ARRAY_SIZE;
    *file_size = FILE_SIZE;
    *num_requests = NUM_REQUESTS;
    *sleeptime = INVERSE_REQUEST_RATE;
}


int main(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr,"Usage: %s <number of server processes>\n", argv[0]);
        exit(0);
    }

    MPI_Init(&argc, &argv);
    int rank, comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    char svr_addr_str[128];
    uint16_t provider_id = 42;
    char filename[128];

    margo_instance_id mid = margo_init("ofi+verbs", MARGO_CLIENT_MODE, 0, 0);

    srand(rank);
    int server_color = rand() % atoi(argv[1]);

    sprintf(filename, "server_addr_%d.txt", server_color);

    MPI_Comm new_comm;
    MPI_Comm_split(MPI_COMM_WORLD, server_color, rank, &new_comm);
    int local_rank;
    MPI_Comm_rank(new_comm, &local_rank);

    FILE * fp;
    fp = fopen(filename, "r");
    fscanf(fp, "%s", svr_addr_str);
    fprintf(stderr, "Server address being read is %s\n", svr_addr_str);

    MPI_Barrier(MPI_COMM_WORLD);

    hg_addr_t svr_addr;
    margo_addr_lookup(mid, svr_addr_str, &svr_addr);

    alpha_client_t alpha_clt;
    alpha_provider_handle_t alpha_ph;

    alpha_client_init(mid, &alpha_clt);

    alpha_provider_handle_create(alpha_clt, svr_addr, provider_id, &alpha_ph);

    int32_t result;
    int32_t transfer_size, compute, memory, file_size, num_requests, sleeptime;

    generate_request_characteristics(&transfer_size, &compute, &memory, &file_size, &num_requests, &sleeptime);

    int32_t * values = (int32_t*)calloc(transfer_size, sizeof(int32_t));
    hg_size_t size = transfer_size*sizeof(int32_t);
    int32_t true_sleeptime;

    hg_bulk_t local_bulk;
    margo_bulk_create(mid, 1, (void**)&values, &size, HG_BULK_READ_ONLY, &local_bulk);

    MPI_Barrier(MPI_COMM_WORLD);

    for(int i=0; i < num_requests; i++) {
      alpha_do_work(alpha_ph, transfer_size, local_bulk, compute, memory, file_size, &result);
    }

    alpha_provider_handle_release(alpha_ph);

    alpha_client_finalize(alpha_clt);

    margo_bulk_free(local_bulk);

    MPI_Barrier(MPI_COMM_WORLD);

    if(!local_rank)
      margo_shutdown_remote_instance(mid, svr_addr); 

    margo_addr_free(mid, svr_addr);

    margo_finalize(mid);

    fclose(fp);

    MPI_Finalize();

    return 0;
}
