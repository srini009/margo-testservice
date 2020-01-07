#include "beta-server.h"
#include "types.h"

struct beta_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

static void beta_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(beta_sum_ult);
static void beta_sum_ult(hg_handle_t h);
/* add other RPC declarations here */

int beta_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        beta_provider_t* provider)
{
    beta_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        fprintf(stderr, "beta_provider_register(): margo instance is not a server.");
        return BETA_FAILURE;
    }

    margo_provider_registered_name(mid, "beta_sum", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "beta_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return BETA_FAILURE;
    }

    p = (beta_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return BETA_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "beta_sum",
            sum_in_t, sum_out_t,
            beta_sum_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &beta_finalize_provider, p);

    //*provider = p;
    return BETA_SUCCESS;
}

static void beta_finalize_provider(void* p)
{
    beta_provider_t provider = (beta_provider_t)p;
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    free(provider);
}

int beta_provider_destroy(
        beta_provider_t provider)
{
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    beta_finalize_provider(provider);

    return BETA_SUCCESS;
}


static void beta_sum_ult(hg_handle_t h)
{
    hg_return_t ret;
    sum_in_t     in;
    sum_out_t   out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    beta_provider_t provider = (beta_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    out.ret = in.x + in.y;
    printf("Computed %d + %d = %d\n",in.x,in.y,out.ret);

    ret = margo_respond(h, &out);

    ret = margo_free_input(h, &in);
}
DEFINE_MARGO_RPC_HANDLER(beta_sum_ult)
