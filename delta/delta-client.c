#include "types.h"
#include "delta-client.h"

struct delta_client {
   margo_instance_id mid;
   hg_id_t           sum_id;
   uint64_t          num_prov_hdl;
};

struct delta_provider_handle {
    delta_client_t client;
    hg_addr_t      addr;
    uint16_t       provider_id;
    uint64_t       refcount;
};

int delta_client_init(margo_instance_id mid, delta_client_t* client)
{
    int ret = DELTA_SUCCESS;

    delta_client_t c = (delta_client_t)calloc(1, sizeof(*c));
    if(!c) return DELTA_FAILURE;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "delta_do_work", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "delta_do_work", &c->sum_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "delta_do_work", delta_in_t, delta_out_t, NULL);
    }

    *client = c;
    return DELTA_SUCCESS;
}

int delta_client_finalize(delta_client_t client)
{
    if(client->num_prov_hdl != 0) {
        fprintf(stderr,  
                "Warning: %d provider handles not released when delta_client_finalize was called\n",
                client->num_prov_hdl);
    }
    free(client);
    return DELTA_SUCCESS;
}

int delta_provider_handle_create(
        delta_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        delta_provider_handle_t* handle)
{
    if(client == DELTA_CLIENT_NULL)
        return DELTA_FAILURE;

    delta_provider_handle_t ph =
        (delta_provider_handle_t)calloc(1, sizeof(*ph));

    if(!ph) return DELTA_FAILURE;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(ph->addr));
    if(ret != HG_SUCCESS) {
        free(ph);
        return DELTA_FAILURE;
    }

    ph->client      = client;
    ph->provider_id = provider_id;
    ph->refcount    = 1;

    client->num_prov_hdl += 1;

    *handle = ph;
    return DELTA_SUCCESS;
}

int delta_provider_handle_ref_incr(
        delta_provider_handle_t handle)
{
    if(handle == DELTA_PROVIDER_HANDLE_NULL)
        return DELTA_FAILURE;
    handle->refcount += 1;
    return DELTA_SUCCESS;
}

int delta_provider_handle_release(delta_provider_handle_t handle)
{
    if(handle == DELTA_PROVIDER_HANDLE_NULL)
        return DELTA_FAILURE;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_prov_hdl -= 1;
        free(handle);
    }
    return DELTA_SUCCESS;
}

int delta_do_work(
        delta_provider_handle_t handle,
        int32_t n,
        hg_bulk_t bulk,
        int32_t* result)
{
    hg_handle_t   h;
    delta_in_t     in;
    delta_out_t   out;
    hg_return_t ret;

    in.n = n;
    in.bulk = bulk;

    ret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(ret != HG_SUCCESS)
        return DELTA_FAILURE;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return DELTA_FAILURE;
    }

    ret = margo_get_output(h, &out);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return DELTA_FAILURE;
    }

    *result = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return DELTA_SUCCESS;
}
