#include <assert.h>
#include "gamma-server.h"
#include "types.h"
#include "delta-client.h"


struct gamma_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

static void gamma_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(gamma_do_work_ult);
static void gamma_do_work_ult(hg_handle_t h);
/* add other RPC declarations here */
delta_client_t delta_clt;
delta_provider_handle_t delta_ph;

int gamma_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        gamma_provider_t* provider)
{
    gamma_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        fprintf(stderr, "gamma_provider_register(): margo instance is not a server.");
        return GAMMA_FAILURE;
    }

    margo_provider_registered_name(mid, "gamma_do_work", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "gamma_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return GAMMA_FAILURE;
    }

    p = (gamma_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return GAMMA_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "gamma_do_work",
            gamma_in_t, gamma_out_t,
            gamma_do_work_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &gamma_finalize_provider, p);

    //*provider = p;
    return GAMMA_SUCCESS;
}

void gamma_create_downstream_handles(margo_instance_id mid, uint16_t p, hg_addr_t svr_addr)
{
    delta_client_init(mid, &delta_clt);
    delta_provider_handle_create(delta_clt, svr_addr, p, &delta_ph);
}

static void gamma_finalize_provider(void* p)
{
    gamma_provider_t provider = (gamma_provider_t)p;
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    free(provider);
}

int gamma_provider_destroy(
        gamma_provider_t provider)
{
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    gamma_finalize_provider(provider);

    delta_provider_handle_release(delta_ph);

    delta_client_finalize(delta_clt);

    return GAMMA_SUCCESS;
}


static void gamma_do_work_ult(hg_handle_t h)
{
    hg_return_t ret;
    gamma_in_t     in;
    gamma_out_t   out;

    int32_t partial_result;
    int32_t* values;
    hg_bulk_t local_bulk;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);

    gamma_provider_t provider = (gamma_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    /* Pull in data from alpha-client through RDMA, simulating a network op */
     values = calloc(in.n, sizeof(*values));
    hg_size_t buf_size = in.n * sizeof(*values);

    ret = margo_bulk_create(mid, 1, (void**)&values, &buf_size,
            HG_BULK_WRITE_ONLY, &local_bulk);
    assert(ret == HG_SUCCESS);

    ret = margo_bulk_transfer(mid, HG_BULK_PULL, info->addr,
            in.bulk, 0, local_bulk, 0, buf_size);
    assert(ret == HG_SUCCESS);

    fprintf(stderr, "Gamma done with it's job.\n");

    ret = margo_bulk_free(local_bulk);
    assert(ret);

    free(values);

    out.ret = 0;

    delta_do_work(delta_ph, in.n, in.bulk, &partial_result);

    ret = margo_respond(h, &out);

    ret = margo_free_input(h, &in);
}
DEFINE_MARGO_RPC_HANDLER(gamma_do_work_ult)
