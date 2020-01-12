#include "delta-server.h"
#include "alpha-client.h"
#include "beta-client.h"
#include "gamma-client.h"
#include "types.h"
#include <assert.h>
#include "../common.h"

struct delta_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

struct rec
{
    int x,y,z;
};

static void delta_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(delta_do_work_ult);
static void delta_do_work_ult(hg_handle_t h);
/* add other RPC declarations here */
beta_client_t beta_clt;
beta_provider_handle_t beta_ph;
alpha_client_t alpha_clt;
alpha_provider_handle_t alpha_ph;
gamma_client_t gamma_clt;
gamma_provider_handle_t gamma_ph;

int delta_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        delta_provider_t* provider)
{
    delta_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        fprintf(stderr, "delta_provider_register(): margo instance is not a server.");
        return DELTA_FAILURE;
    }

    margo_provider_registered_name(mid, "delta_do_work", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "delta_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return DELTA_FAILURE;
    }

    p = (delta_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return DELTA_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "delta_do_work",
            delta_in_t, delta_out_t,
            delta_do_work_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &delta_finalize_provider, p);

    return DELTA_SUCCESS;
}

void delta_create_downstream_handles(margo_instance_id mid, uint16_t p, hg_addr_t svr_addr)
{
    gamma_client_init(mid, &gamma_clt);
    gamma_provider_handle_create(gamma_clt, svr_addr, p, &gamma_ph);
    beta_client_init(mid, &beta_clt);
    beta_provider_handle_create(beta_clt, svr_addr, p, &beta_ph);
    alpha_client_init(mid, &alpha_clt);
    alpha_provider_handle_create(alpha_clt, svr_addr, p, &alpha_ph);
}

static void delta_finalize_provider(void* p)
{
    delta_provider_t provider = (delta_provider_t)p;
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    free(provider);
}

int delta_provider_destroy(
        delta_provider_t provider)
{
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    delta_finalize_provider(provider);

    alpha_provider_handle_release(alpha_ph);
    alpha_client_finalize(alpha_clt);
    beta_provider_handle_release(beta_ph);
    beta_client_finalize(beta_clt);
    gamma_provider_handle_release(gamma_ph);
    gamma_client_finalize(gamma_clt);

    return DELTA_SUCCESS;
}


static void delta_do_work_ult(hg_handle_t h)
{
    hg_return_t ret;
    delta_in_t     in;
    delta_out_t   out;

    struct rec r;

    FILE *fp = fopen("/dev/shm/junk", "w");
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    delta_provider_t provider = (delta_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    /* Do I/O work */
    for(int i = 0; i < in.file_size; i++) {
      r.x = i;
      fwrite(&r,sizeof(struct rec),1,fp);
    }

    remove("/dev/shm/junk");
    fclose(fp);

    //fprintf(stderr, "Delta is done with its job.\n");

    out.ret = 0;

    ret = margo_respond(h, &out);
    assert(ret == HG_SUCCESS);

    ret = margo_free_input(h, &in);
    assert(ret == HG_SUCCESS);

    ret = margo_destroy(h);
    assert(ret == HG_SUCCESS);
}
DEFINE_MARGO_RPC_HANDLER(delta_do_work_ult)
