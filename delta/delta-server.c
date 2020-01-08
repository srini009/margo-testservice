#include "delta-server.h"
#include "types.h"

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
    for(int i = 0; i < 100000000; i++) {
      r.x = i;
      fwrite(&r,sizeof(struct rec),1,fp);
    }

    //remove("/dev/shm/junk");
    fclose(fp);
    fprintf(stderr, "Delta is done with its job.\n");

    out.ret = 0;

    ret = margo_respond(h, &out);

    ret = margo_free_input(h, &in);
}
DEFINE_MARGO_RPC_HANDLER(delta_do_work_ult)
