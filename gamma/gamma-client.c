#include "types.h"
#include "gamma-client.h"

struct gamma_client {
   margo_instance_id mid;
   hg_id_t           sum_id;
   uint64_t          num_prov_hdl;
};

struct gamma_provider_handle {
    gamma_client_t client;
    hg_addr_t      addr;
    uint16_t       provider_id;
    uint64_t       refcount;
};

int gamma_client_init(margo_instance_id mid, gamma_client_t* client)
{
    int ret = GAMMA_SUCCESS;

    gamma_client_t c = (gamma_client_t)calloc(1, sizeof(*c));
    if(!c) return GAMMA_FAILURE;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "gamma_sum", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "gamma_sum", &c->sum_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "gamma_sum", sum_in_t, sum_out_t, NULL);
    }

    *client = c;
    return GAMMA_SUCCESS;
}

int gamma_client_finalize(gamma_client_t client)
{
    if(client->num_prov_hdl != 0) {
        fprintf(stderr,  
                "Warning: %d provider handles not released when gamma_client_finalize was called\n",
                client->num_prov_hdl);
    }
    free(client);
    return GAMMA_SUCCESS;
}

int gamma_provider_handle_create(
        gamma_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        gamma_provider_handle_t* handle)
{
    if(client == GAMMA_CLIENT_NULL)
        return GAMMA_FAILURE;

    gamma_provider_handle_t ph =
        (gamma_provider_handle_t)calloc(1, sizeof(*ph));

    if(!ph) return GAMMA_FAILURE;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(ph->addr));
    if(ret != HG_SUCCESS) {
        free(ph);
        return GAMMA_FAILURE;
    }

    ph->client      = client;
    ph->provider_id = provider_id;
    ph->refcount    = 1;

    client->num_prov_hdl += 1;

    *handle = ph;
    return GAMMA_SUCCESS;
}

int gamma_provider_handle_ref_incr(
        gamma_provider_handle_t handle)
{
    if(handle == GAMMA_PROVIDER_HANDLE_NULL)
        return GAMMA_FAILURE;
    handle->refcount += 1;
    return GAMMA_SUCCESS;
}

int gamma_provider_handle_release(gamma_provider_handle_t handle)
{
    if(handle == GAMMA_PROVIDER_HANDLE_NULL)
        return GAMMA_FAILURE;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_prov_hdl -= 1;
        free(handle);
    }
    return GAMMA_SUCCESS;
}

int gamma_compute_sum(
        gamma_provider_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result)
{
    hg_handle_t   h;
    sum_in_t     in;
    sum_out_t   out;
    hg_return_t ret;

    in.x = x;
    in.y = y;

    ret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(ret != HG_SUCCESS)
        return GAMMA_FAILURE;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return GAMMA_FAILURE;
    }

    ret = margo_get_output(h, &out);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return GAMMA_FAILURE;
    }

    *result = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return GAMMA_SUCCESS;
}
