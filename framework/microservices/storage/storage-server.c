#include <assert.h>
#include "../common.h"
#include "storage-server.h"
#include "network-client.h"
#include "memory-client.h"
#include "compute-client.h"

struct storage_provider {
    margo_instance_id mid;
    hg_id_t sum_id;
    /* other provider-specific data */
};

struct rec
{
    int x,y,z;
};

static void storage_finalize_provider(void* p);

DECLARE_MARGO_RPC_HANDLER(storage_do_work_ult);
static void storage_do_work_ult(hg_handle_t h);
/* add other RPC declarations here */

int storage_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        storage_provider_t* provider)
{
    storage_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        fprintf(stderr, "storage_provider_register(): margo instance is not a server.");
        return STORAGE_FAILURE;
    }

    margo_provider_registered_name(mid, "storage_do_work", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        fprintf(stderr, "storage_provider_register(): a provider with the same provider id (%d) already exists.\n", provider_id);
        return STORAGE_FAILURE;
    }

    p = (storage_provider_t)calloc(1, sizeof(*p));
    if(p == NULL)
        return STORAGE_FAILURE;

    p->mid = mid;

    id = MARGO_REGISTER_PROVIDER(mid, "storage_do_work",
            symbio_in_t, symbio_out_t,
            storage_do_work_ult, provider_id, pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;
    /* add other RPC registration here */

    margo_provider_push_finalize_callback(mid, p, &storage_finalize_provider, p);

    return STORAGE_SUCCESS;
}

static void storage_finalize_provider(void* p)
{
    storage_provider_t provider = (storage_provider_t)p;
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    free(provider);
}

int storage_provider_destroy(
        storage_provider_t provider)
{
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    storage_finalize_provider(provider);

    return STORAGE_SUCCESS;
}


static void storage_do_work_ult(hg_handle_t h)
{
    hg_return_t ret;
    symbio_in_t     in;
    symbio_out_t   out;

    struct rec r;

    FILE *fp = fopen("/dev/shm/junk", "w");
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    const struct hg_info* info = margo_get_info(h);
    storage_provider_t provider = (storage_provider_t)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);

    /* Do I/O work */
    for(int i = 0; i < in.workload_factor * FILE_SIZE; i++) {
      r.x = i;
      fwrite(&r,sizeof(struct rec),1,fp);
    }

    remove("/dev/shm/junk");
    fclose(fp);

    out.ret = 0;

    ret = margo_respond(h, &out);
    assert(ret == HG_SUCCESS);

    ret = margo_free_input(h, &in);
    assert(ret == HG_SUCCESS);

    ret = margo_destroy(h);
    assert(ret == HG_SUCCESS);
}
DEFINE_MARGO_RPC_HANDLER(storage_do_work_ult)
