#include <assert.h>
#include "network-server.h"
#include "memory-client.h"
#include "compute-client.h"
#include "storage-client.h"
#include "../../include/types.h"
#include "../common.h"

struct network_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

static void network_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(network_do_work_ult);
static void network_do_work_ult(hg_handle_t h);

memory_client_t memory_clt;
memory_provider_handle_t memory_ph;
compute_client_t compute_clt;
compute_provider_handle_t compute_ph;
storage_client_t storage_clt;
storage_provider_handle_t storage_ph;

/* add other RPC declarations here */

int network_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        network_provider_t* provider)
{
    network_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        fprintf(stderr, "network_provider_register(): margo instance is not a server.");
        return NETWORK_FAILURE;
    }

    margo_provider_registered_name(mid, "network_sum", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "network_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return NETWORK_FAILURE;
    }

    p = (network_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return NETWORK_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "network_do_work",
            network_in_t, network_out_t,
            network_do_work_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &network_finalize_provider, p);

    return NETWORK_SUCCESS;
}

void network_create_downstream_handles(margo_instance_id mid, uint16_t p, hg_addr_t svr_addr)
{
    memory_client_init(mid, &memory_clt);
    memory_provider_handle_create(memory_clt, svr_addr, p, &memory_ph);
    compute_client_init(mid, &compute_clt);
    compute_provider_handle_create(compute_clt, svr_addr, p, &compute_ph);
    storage_client_init(mid, &storage_clt);
    storage_provider_handle_create(storage_clt, svr_addr, p, &storage_ph);
}

static void network_finalize_provider(void* p)
{
    network_provider_t provider = (network_provider_t)p;
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    free(provider);
}

int network_provider_destroy(
        network_provider_t provider)
{
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    network_finalize_provider(provider);

    memory_provider_handle_release(memory_ph);
    memory_client_finalize(memory_clt);
    compute_provider_handle_release(compute_ph);
    compute_client_finalize(compute_clt);
    storage_provider_handle_release(storage_ph);
    storage_client_finalize(storage_clt);
 

    return NETWORK_SUCCESS;
}


static void network_do_work_ult(hg_handle_t h)
{
    hg_return_t ret;
    network_in_t     in;
    network_out_t   out;
    int32_t partial_result;
    int32_t* values;
    hg_bulk_t local_bulk;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    network_provider_t provider = (network_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    out.ret = 0;

    /* Pull in data from network-client through RDMA, simulating a network op */
    values = calloc(in.n, sizeof(*values));
    hg_size_t buf_size = in.n * sizeof(*values);

    ret = margo_bulk_create(mid, 1, (void**)&values, &buf_size,
            HG_BULK_WRITE_ONLY, &local_bulk);
    assert(ret == HG_SUCCESS);

    ret = margo_bulk_transfer(mid, HG_BULK_PULL, info->addr,
            in.bulk, 0, local_bulk, 0, buf_size);
    assert(ret == HG_SUCCESS);

    memory_do_work(memory_ph, in.n, in.bulk, in.compute, in.memory, in.file_size, &partial_result);

    ret = margo_respond(h, &out);
    assert(ret == HG_SUCCESS);
    

    ret = margo_bulk_free(local_bulk);
    free(values);
 
    ret = margo_free_input(h, &in);
    assert(ret == HG_SUCCESS);

    ret = margo_destroy(h);
    assert(ret == HG_SUCCESS);

}
DEFINE_MARGO_RPC_HANDLER(network_do_work_ult)
