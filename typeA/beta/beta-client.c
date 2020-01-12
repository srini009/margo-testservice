#include "types.h"
#include "beta-client.h"

struct beta_client {
   margo_instance_id mid;
   hg_id_t           sum_id;
   uint64_t          num_prov_hdl;
};

struct beta_provider_handle {
    beta_client_t client;
    hg_addr_t      addr;
    uint16_t       provider_id;
    uint64_t       refcount;
};

int beta_client_init(margo_instance_id mid, beta_client_t* client)
{
    int ret = BETA_SUCCESS;

    beta_client_t c = (beta_client_t)calloc(1, sizeof(*c));
    if(!c) return BETA_FAILURE;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "beta_do_work", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "beta_do_work", &c->sum_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "beta_do_work", beta_in_t, beta_out_t, NULL);
    }

    *client = c;
    return BETA_SUCCESS;
}

int beta_client_finalize(beta_client_t client)
{
    if(client->num_prov_hdl != 0) {
        fprintf(stderr,  
                "Warning: %d provider handles not released when beta_client_finalize was called\n",
                client->num_prov_hdl);
    }
    free(client);
    return BETA_SUCCESS;
}

int beta_provider_handle_create(
        beta_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        beta_provider_handle_t* handle)
{
    if(client == BETA_CLIENT_NULL)
        return BETA_FAILURE;

    beta_provider_handle_t ph =
        (beta_provider_handle_t)calloc(1, sizeof(*ph));

    if(!ph) return BETA_FAILURE;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(ph->addr));
    if(ret != HG_SUCCESS) {
        free(ph);
        return BETA_FAILURE;
    }

    ph->client      = client;
    ph->provider_id = provider_id;
    ph->refcount    = 1;

    client->num_prov_hdl += 1;

    *handle = ph;
    return BETA_SUCCESS;
}

int beta_provider_handle_ref_incr(
        beta_provider_handle_t handle)
{
    if(handle == BETA_PROVIDER_HANDLE_NULL)
        return BETA_FAILURE;
    handle->refcount += 1;
    return BETA_SUCCESS;
}

int beta_provider_handle_release(beta_provider_handle_t handle)
{
    if(handle == BETA_PROVIDER_HANDLE_NULL)
        return BETA_FAILURE;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_prov_hdl -= 1;
        free(handle);
    }
    return BETA_SUCCESS;
}

int beta_do_work(
        beta_provider_handle_t handle,
        int32_t n,
        hg_bulk_t bulk,
        int32_t* result)
{
    hg_handle_t   h;
    beta_in_t     in;
    beta_out_t   out;
    hg_return_t ret;

    in.n = n;
    in.bulk = bulk;

    ret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(ret != HG_SUCCESS)
        return BETA_FAILURE;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return BETA_FAILURE;
    }

    ret = margo_get_output(h, &out);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return BETA_FAILURE;
    }

    *result = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return BETA_SUCCESS;
}
