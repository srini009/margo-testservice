#include <assert.h>
#include "../common.h"
#include "network-server.h"
#include "memory-client.h"
#include "compute-client.h"
#include "storage-client.h"

struct network_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

static void network_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(network_do_work_ult);
static void network_do_work_ult(hg_handle_t h);

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
            symbio_in_t, symbio_out_t,
            network_do_work_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &network_finalize_provider, p);

    return NETWORK_SUCCESS;
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

    return NETWORK_SUCCESS;
}

static void network_do_work_ult(hg_handle_t h)
{
    hg_return_t ret;
    symbio_in_t     in;
    symbio_out_t   out;
    int32_t partial_result;
    int32_t* values;
    hg_bulk_t local_bulk;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    network_provider_t provider = (network_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    out.ret = 0;

    /* Pull in data from network-client through RDMA, simulating a network op */
    values = calloc(in.workload_factor * TRANSFER_SIZE, sizeof(*values));
    hg_size_t buf_size = in.workload_factor * TRANSFER_SIZE * sizeof(*values);

    ret = margo_bulk_create(mid, 1, (void**)&values, &buf_size,
            HG_BULK_WRITE_ONLY, &local_bulk);
    assert(ret == HG_SUCCESS);

    ret = margo_bulk_transfer(mid, HG_BULK_PULL, info->addr,
            in.bulk, 0, local_bulk, 0, buf_size);
    assert(ret == HG_SUCCESS);

    memory_do_work(GENERATE_PROVIDER_HANDLE(dummy, memory, 0), in.workload_factor, in.bulk, in.request_structure, &partial_result);

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
