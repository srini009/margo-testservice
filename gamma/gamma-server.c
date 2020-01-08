#include "gamma-server.h"
#include "types.h"
#include "delta-client.h"

#define ARRAY_SIZE 1000000000

struct gamma_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

static void gamma_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(gamma_sum_ult);
static void gamma_sum_ult(hg_handle_t h);
/* add other RPC declarations here */
int *a, *c;
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

    margo_provider_registered_name(mid, "gamma_sum", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "gamma_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return GAMMA_FAILURE;
    }

    p = (gamma_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return GAMMA_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "gamma_sum",
            sum_in_t, sum_out_t,
            gamma_sum_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &gamma_finalize_provider, p);

    a = (int*)malloc(ARRAY_SIZE*sizeof(int));
    c = (int*)malloc(ARRAY_SIZE*sizeof(int));

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
    free(a);
    free(c);

    delta_provider_handle_release(delta_ph);

    delta_client_finalize(delta_clt);

    return GAMMA_SUCCESS;
}


static void gamma_sum_ult(hg_handle_t h)
{
    hg_return_t ret;
    sum_in_t     in;
    sum_out_t   out;

    int32_t partial_result; 

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    gamma_provider_t provider = (gamma_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    for(int i = 0 ; i < ARRAY_SIZE; i++)
      c[i] = a[i];

    fprintf(stderr, "Gamma done with it's job.\n");

    out.ret = in.x + in.y;

    delta_compute_sum(delta_ph, 1, 1, &partial_result);

    ret = margo_respond(h, &out);

    ret = margo_free_input(h, &in);
}
DEFINE_MARGO_RPC_HANDLER(gamma_sum_ult)
