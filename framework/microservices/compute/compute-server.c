#include <assert.h>
#include "compute-server.h"
#include "../../include/types.h"
#include "memory-client.h"
#include "network-client.h"
#include "storage-client.h"
#include "../common.h"

struct compute_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

static void compute_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(compute_do_work_ult);
static void compute_do_work_ult(hg_handle_t h);
/* add other RPC declarations here */
memory_client_t memory_clt;
memory_provider_handle_t memory_ph;
network_client_t network_clt;
network_provider_handle_t network_ph;
storage_client_t storage_clt;
storage_provider_handle_t storage_ph;

int compute_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        compute_provider_t* provider)
{
    compute_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        fprintf(stderr, "compute_provider_register(): margo instance is not a server.");
        return COMPUTE_FAILURE;
    }

    margo_provider_registered_name(mid, "compute_do_work", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "compute_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return COMPUTE_FAILURE;
    }

    p = (compute_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return COMPUTE_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "compute_do_work",
            compute_in_t, compute_out_t,
            compute_do_work_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &compute_finalize_provider, p);

    //*provider = p;
    return COMPUTE_SUCCESS;
}

void compute_create_downstream_handles(margo_instance_id mid, uint16_t p, hg_addr_t svr_addr)
{
    storage_client_init(mid, &storage_clt);
    storage_provider_handle_create(storage_clt, svr_addr, p, &storage_ph);
    memory_client_init(mid, &memory_clt);
    memory_provider_handle_create(memory_clt, svr_addr, p, &memory_ph);
    network_client_init(mid, &network_clt);
    network_provider_handle_create(network_clt, svr_addr, p, &network_ph);
}

static void compute_finalize_provider(void* p)
{
    compute_provider_t provider = (compute_provider_t)p;
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    free(provider);
}

int compute_provider_destroy(
        compute_provider_t provider)
{
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    compute_finalize_provider(provider);

    network_provider_handle_release(network_ph);
    network_client_finalize(network_clt);
    memory_provider_handle_release(memory_ph);
    memory_client_finalize(memory_clt);
    storage_provider_handle_release(storage_ph);
    storage_client_finalize(storage_clt);

    return COMPUTE_SUCCESS;
}


static void compute_do_work_ult(hg_handle_t h)
{
    hg_return_t ret;
    compute_in_t     in;
    compute_out_t   out;

    int32_t partial_result;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);

    compute_provider_t provider = (compute_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    /* Bogus CPU-bound computation */
    for (int i = 0 ; i < in.compute; i++)
      out.ret = out.ret + (45 + 69)*2 + i;

    storage_do_work(storage_ph, in.n, in.bulk, in.compute, in.memory, in.file_size, &partial_result);

    ret = margo_respond(h, &out);
    assert(ret == HG_SUCCESS);

    ret = margo_free_input(h, &in);
    assert(ret == HG_SUCCESS);

    ret = margo_destroy(h);
    assert(ret == HG_SUCCESS);
}
DEFINE_MARGO_RPC_HANDLER(compute_do_work_ult)
