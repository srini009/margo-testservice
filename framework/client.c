#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <mpi.h>
#include <unistd.h>
#include "network-client.h"
#include "include/defaults.h"

#include "dummy_client.h"

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
    uint16_t provider_id = 0;
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

    network_client_t network_clt;
    network_provider_handle_t network_ph;

    network_client_init(mid, &network_clt);

    network_provider_handle_create(network_clt, svr_addr, provider_id, &network_ph);

    int32_t result;

    int32_t * values = (int32_t*)calloc(TRANSFER_SIZE, sizeof(int32_t));
    hg_size_t size = TRANSFER_SIZE*sizeof(int32_t);
    int32_t true_sleeptime;
    hg_string_t request_structure = get_dummy_service_request_structure(testing2);

    fprintf(stderr, "Request structure is: %s\n", request_structure);

    hg_bulk_t local_bulk;
    margo_bulk_create(mid, 1, (void**)&values, &size, HG_BULK_READ_ONLY, &local_bulk);

    MPI_Barrier(MPI_COMM_WORLD);
    for(int i=0; i < NUM_REQUESTS; i++) {
      network_do_work(network_ph, 1, local_bulk, request_structure, &result);
    }

    network_provider_handle_release(network_ph);

    network_client_finalize(network_clt);

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
