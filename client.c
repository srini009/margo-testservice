#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include "alpha-client.h"
#include "common.h"
#include <mpi.h>

int main(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr,"Usage: %s <number of server processes>\n", argv[0]);
        exit(0);
    }

    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char svr_addr_str[128];
    uint16_t provider_id = 42;
    char filename[128];

    margo_instance_id mid = margo_init("ofi+verbs", MARGO_CLIENT_MODE, 0, 0);

    srand(rank);
    int server = rand() % atoi(argv[1]);

    sprintf(filename, "server_addr_%d.txt", server);

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

    int32_t * values = (int32_t*)calloc(TRANSFER_SIZE, sizeof(int32_t));
    hg_size_t size = TRANSFER_SIZE*sizeof(int32_t);

    hg_bulk_t local_bulk;
    margo_bulk_create(mid, 1, (void**)&values, &size, HG_BULK_READ_ONLY, &local_bulk);

    for(int i=0; i < 100; i++)
      alpha_do_work(alpha_ph, TRANSFER_SIZE, local_bulk, &result);

    alpha_provider_handle_release(alpha_ph);

    alpha_client_finalize(alpha_clt);

    margo_bulk_free(local_bulk);

    if(!rank)
      margo_shutdown_remote_instance(mid, svr_addr); 

    margo_addr_free(mid, svr_addr);

    margo_finalize(mid);

    fclose(fp);

    MPI_Finalize();

    return 0;
}
