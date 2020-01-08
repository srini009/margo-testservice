#include <assert.h>
#include "beta-server.h"
#include "types.h"
#include "gamma-client.h"

#define ARRAY_SIZE 1000000000

struct beta_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

gamma_client_t gamma_clt;
gamma_provider_handle_t gamma_ph;

static void beta_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(beta_do_work_ult);
static void beta_do_work_ult(hg_handle_t h);
/* add other RPC declarations here */
int *a, *c;

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

    margo_provider_registered_name(mid, "beta_do_work", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "beta_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return BETA_FAILURE;
    }

    p = (beta_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return BETA_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "beta_do_work",
            beta_in_t, beta_out_t,
            beta_do_work_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &beta_finalize_provider, p);

    a = (int*)malloc(ARRAY_SIZE*sizeof(int));
    c = (int*)malloc(ARRAY_SIZE*sizeof(int));

    //*provider = p;
    return BETA_SUCCESS;
}

void beta_create_downstream_handles(margo_instance_id mid, uint16_t p, hg_addr_t svr_addr)
{
    gamma_client_init(mid, &gamma_clt);
    gamma_provider_handle_create(gamma_clt, svr_addr, p, &gamma_ph);
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

    gamma_provider_handle_release(gamma_ph);

    gamma_client_finalize(gamma_clt);
    free(a);
    free(c);

    return BETA_SUCCESS;
}


static void beta_do_work_ult(hg_handle_t h)
{
    hg_return_t ret;
    beta_in_t     in;
    beta_out_t   out;
    
    int32_t partial_result;

    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    beta_provider_t provider = (beta_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    /* Dummy memory-bound operation */
    for(int i = 0 ; i < ARRAY_SIZE; i++)
      c[i] = a[i];

    fprintf(stderr, "Beta done with it's job.\n");

    out.ret = 0;

    gamma_do_work(gamma_ph, in.n, in.bulk, &partial_result);

    ret = margo_respond(h, &out);

    ret = margo_free_input(h, &in);
}
DEFINE_MARGO_RPC_HANDLER(beta_do_work_ult)
