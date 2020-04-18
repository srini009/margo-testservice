#include "../../include/types.h"
#include "storage-client.h"

struct storage_client {
   margo_instance_id mid;
   hg_id_t           sum_id;
   uint64_t          num_prov_hdl;
};

struct storage_provider_handle {
    storage_client_t client;
    hg_addr_t      addr;
    uint16_t       provider_id;
    uint64_t       refcount;
};

int storage_client_init(margo_instance_id mid, storage_client_t* client)
{
    int ret = STORAGE_SUCCESS;

    storage_client_t c = (storage_client_t)calloc(1, sizeof(*c));
    if(!c) return STORAGE_FAILURE;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "storage_do_work", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "storage_do_work", &c->sum_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "storage_do_work", storage_in_t, storage_out_t, NULL);
    }

    *client = c;
    return STORAGE_SUCCESS;
}

int storage_client_finalize(storage_client_t client)
{
    if(client->num_prov_hdl != 0) {
        fprintf(stderr,  
                "Warning: %d provider handles not released when storage_client_finalize was called\n",
                client->num_prov_hdl);
    }
    free(client);
    return STORAGE_SUCCESS;
}

int storage_provider_handle_create(
        storage_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        storage_provider_handle_t* handle)
{
    if(client == STORAGE_CLIENT_NULL)
        return STORAGE_FAILURE;

    storage_provider_handle_t ph =
        (storage_provider_handle_t)calloc(1, sizeof(*ph));

    if(!ph) return STORAGE_FAILURE;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(ph->addr));
    if(ret != HG_SUCCESS) {
        free(ph);
        return STORAGE_FAILURE;
    }

    ph->client      = client;
    ph->provider_id = provider_id;
    ph->refcount    = 1;

    client->num_prov_hdl += 1;

    *handle = ph;
    return STORAGE_SUCCESS;
}

int storage_provider_handle_ref_incr(
        storage_provider_handle_t handle)
{
    if(handle == STORAGE_PROVIDER_HANDLE_NULL)
        return STORAGE_FAILURE;
    handle->refcount += 1;
    return STORAGE_SUCCESS;
}

int storage_provider_handle_release(storage_provider_handle_t handle)
{
    if(handle == STORAGE_PROVIDER_HANDLE_NULL)
        return STORAGE_FAILURE;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_prov_hdl -= 1;
        free(handle);
    }
    return STORAGE_SUCCESS;
}

int storage_do_work(
        storage_provider_handle_t handle,
        int32_t n,
        hg_bulk_t bulk,
        int32_t compute,
        int32_t memory,
        int32_t file_size,
        int32_t* result)
{
    hg_handle_t   h;
    storage_in_t     in;
    storage_out_t   out;
    hg_return_t ret;

    in.n = n;
    in.bulk = bulk;
    in.compute = compute;
    in.memory = memory;
    in.file_size = file_size;

    ret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(ret != HG_SUCCESS)
        return STORAGE_FAILURE;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return STORAGE_FAILURE;
    }

    ret = margo_get_output(h, &out);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return STORAGE_FAILURE;
    }

    *result = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return STORAGE_SUCCESS;
}
