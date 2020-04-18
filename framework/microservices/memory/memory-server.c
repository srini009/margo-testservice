#include <assert.h>
#include "memory-server.h"
#include "../../include/types.h"
#include "compute-client.h"
#include "network-client.h"
#include "storage-client.h"
#include "../common.h"

struct memory_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

compute_client_t compute_clt;
compute_provider_handle_t compute_ph;
network_client_t network_clt;
network_provider_handle_t network_ph;
storage_client_t storage_clt;
storage_provider_handle_t storage_ph;

static void memory_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(memory_do_work_ult);
static void memory_do_work_ult(hg_handle_t h);
/* add other RPC declarations here */
int *a, *c;

int memory_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        memory_provider_t* provider)
{
    memory_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        fprintf(stderr, "memory_provider_register(): margo instance is not a server.");
        return MEMORY_FAILURE;
    }

    margo_provider_registered_name(mid, "memory_do_work", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "memory_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return MEMORY_FAILURE;
    }

    p = (memory_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return MEMORY_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "memory_do_work",
            memory_in_t, memory_out_t,
            memory_do_work_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &memory_finalize_provider, p);

    a = (int*)malloc(1000000000*sizeof(int));
    c = (int*)malloc(1000000000*sizeof(int));

    //*provider = p;
    return MEMORY_SUCCESS;
}

void memory_create_downstream_handles(margo_instance_id mid, uint16_t p, hg_addr_t svr_addr)
{
    compute_client_init(mid, &compute_clt);
    compute_provider_handle_create(compute_clt, svr_addr, p, &compute_ph);
    network_client_init(mid, &network_clt);
    network_provider_handle_create(network_clt, svr_addr, p, &network_ph);
    storage_client_init(mid, &storage_clt);
    storage_provider_handle_create(storage_clt, svr_addr, p, &storage_ph);
}

static void memory_finalize_provider(void* p)
{
    memory_provider_t provider = (memory_provider_t)p;
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    free(provider);
}

int memory_provider_destroy(
        memory_provider_t provider)
{
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    memory_finalize_provider(provider);

    network_provider_handle_release(network_ph);
    network_client_finalize(network_clt);
    compute_provider_handle_release(compute_ph);
    compute_client_finalize(compute_clt);
    storage_provider_handle_release(storage_ph);
    storage_client_finalize(storage_clt);

    free(a);
    free(c);

    return MEMORY_SUCCESS;
}


static void memory_do_work_ult(hg_handle_t h)
{
    hg_return_t ret;
    memory_in_t     in;
    memory_out_t   out;
    
    int32_t partial_result;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    memory_provider_t provider = (memory_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    /* Dummy memory-bound operation */
    for(int i = 0 ; i < in.memory; i++)
      c[i] = a[i];

    out.ret = 0;

    compute_do_work(compute_ph, in.n, in.bulk, in.compute, in.memory, in.file_size, &partial_result);

    ret = margo_respond(h, &out);
    assert(ret == HG_SUCCESS);

    ret = margo_free_input(h, &in);
    assert(ret == HG_SUCCESS);

    ret = margo_destroy(h);
    assert(ret == HG_SUCCESS);
}
DEFINE_MARGO_RPC_HANDLER(memory_do_work_ult)