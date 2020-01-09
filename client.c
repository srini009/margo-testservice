#include <stdio.h>
#include <margo.h>
#include "alpha-client.h"

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr,"Usage: %s <server address> <provider id>\n", argv[0]);
        exit(0);
    }

    const char* svr_addr_str = argv[1];
    uint16_t    provider_id  = atoi(argv[2]);

    margo_instance_id mid = margo_init("ofi+verbs", MARGO_CLIENT_MODE, 0, 0);

    hg_addr_t svr_addr;
    margo_addr_lookup(mid, svr_addr_str, &svr_addr);

    alpha_client_t alpha_clt;
    alpha_provider_handle_t alpha_ph;

    alpha_client_init(mid, &alpha_clt);

    alpha_provider_handle_create(alpha_clt, svr_addr, provider_id, &alpha_ph);

    int32_t result;

    int32_t * values = (int32_t*)calloc(1000, sizeof(int32_t));
    hg_size_t size = 1000*sizeof(int32_t);

    hg_bulk_t local_bulk;
    margo_bulk_create(mid, 1, (void**)&values, &size, HG_BULK_READ_ONLY, &local_bulk);
    alpha_do_work(alpha_ph, 1000, local_bulk, &result);

    alpha_provider_handle_release(alpha_ph);

    alpha_client_finalize(alpha_clt);

    margo_bulk_free(local_bulk);

    margo_shutdown_remote_instance(mid, svr_addr); 

    margo_addr_free(mid, svr_addr);

    margo_finalize(mid);

    return 0;
}
