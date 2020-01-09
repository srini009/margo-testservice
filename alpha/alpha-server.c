#include <assert.h>
#include "alpha-server.h"
#include "beta-client.h"
#include "types.h"
#include "../common.h"

struct alpha_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

static void alpha_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(alpha_do_work_ult);
static void alpha_do_work_ult(hg_handle_t h);

beta_client_t beta_clt;
beta_provider_handle_t beta_ph;

/* add other RPC declarations here */

int alpha_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        alpha_provider_t* provider)
{
    alpha_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        fprintf(stderr, "alpha_provider_register(): margo instance is not a server.");
        return ALPHA_FAILURE;
    }

    margo_provider_registered_name(mid, "alpha_sum", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "alpha_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return ALPHA_FAILURE;
    }

    p = (alpha_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return ALPHA_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "alpha_do_work",
            alpha_in_t, alpha_out_t,
            alpha_do_work_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &alpha_finalize_provider, p);

    return ALPHA_SUCCESS;
}

void alpha_create_downstream_handles(margo_instance_id mid, uint16_t p, hg_addr_t svr_addr)
{
    beta_client_init(mid, &beta_clt);
    beta_provider_handle_create(beta_clt, svr_addr, p, &beta_ph);
}

static void alpha_finalize_provider(void* p)
{
    alpha_provider_t provider = (alpha_provider_t)p;
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    free(provider);
}

int alpha_provider_destroy(
        alpha_provider_t provider)
{
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    alpha_finalize_provider(provider);

    beta_provider_handle_release(beta_ph);

    beta_client_finalize(beta_clt);
 

    return ALPHA_SUCCESS;
}


static void alpha_do_work_ult(hg_handle_t h)
{
    hg_return_t ret;
    alpha_in_t     in;
    alpha_out_t   out;
    int32_t partial_result;
    int32_t* values;
    hg_bulk_t local_bulk;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    alpha_provider_t provider = (alpha_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    out.ret = 0;

    /* Pull in data from alpha-client through RDMA, simulating a network op */
    values = calloc(in.n, sizeof(*values));
    hg_size_t buf_size = in.n * sizeof(*values);

    ret = margo_bulk_create(mid, 1, (void**)&values, &buf_size,
            HG_BULK_WRITE_ONLY, &local_bulk);
    assert(ret == HG_SUCCESS);

    ret = margo_bulk_transfer(mid, HG_BULK_PULL, info->addr,
            in.bulk, 0, local_bulk, 0, buf_size);
    assert(ret == HG_SUCCESS);

    //fprintf(stderr, "Alpha done with it's job.\n");

    beta_do_work(beta_ph, in.n, in.bulk, &partial_result);
    

    ret = margo_respond(h, &out);
    assert(ret == HG_SUCCESS);
    

    ret = margo_bulk_free(local_bulk);
    free(values);
 
    ret = margo_free_input(h, &in);
    assert(ret == HG_SUCCESS);

    ret = margo_destroy(h);
    assert(ret == HG_SUCCESS);

}
DEFINE_MARGO_RPC_HANDLER(alpha_do_work_ult)
