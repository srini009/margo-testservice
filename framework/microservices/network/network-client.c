#include "network-client.h"
#include "../../include/defaults.h"

struct network_client {
   margo_instance_id mid;
   hg_id_t           sum_id;
   uint64_t          num_prov_hdl;
};

struct network_provider_handle {
    network_client_t client;
    hg_addr_t      addr;
    uint16_t       provider_id;
    uint64_t       refcount;
};

int network_client_init(margo_instance_id mid, network_client_t* client)
{
    int ret = NETWORK_SUCCESS;

    network_client_t c = (network_client_t)calloc(1, sizeof(*c));
    if(!c) return NETWORK_FAILURE;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "network_do_work", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "network_do_work", &c->sum_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "network_do_work", symbio_in_t, symbio_out_t, NULL);
    }

    *client = c;
    return NETWORK_SUCCESS;
}

int network_client_finalize(network_client_t client)
{
    if(client->num_prov_hdl != 0) {
        fprintf(stderr,  
                "Warning: %d provider handles not released when network_client_finalize was called\n",
                client->num_prov_hdl);
    }
    free(client);
    return NETWORK_SUCCESS;
}

int network_provider_handle_create(
        network_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        network_provider_handle_t* handle)
{
    if(client == NETWORK_CLIENT_NULL)
        return NETWORK_FAILURE;

    network_provider_handle_t ph =
        (network_provider_handle_t)calloc(1, sizeof(*ph));

    if(!ph) return NETWORK_FAILURE;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(ph->addr));
    if(ret != HG_SUCCESS) {
        free(ph);
        return NETWORK_FAILURE;
    }

    ph->client      = client;
    ph->provider_id = provider_id;
    ph->refcount    = 1;

    client->num_prov_hdl += 1;

    *handle = ph;
    return NETWORK_SUCCESS;
}

int network_provider_handle_ref_incr(
        network_provider_handle_t handle)
{
    if(handle == NETWORK_PROVIDER_HANDLE_NULL)
        return NETWORK_FAILURE;
    handle->refcount += 1;
    return NETWORK_SUCCESS;
}

int network_provider_handle_release(network_provider_handle_t handle)
{
    if(handle == NETWORK_PROVIDER_HANDLE_NULL)
        return NETWORK_FAILURE;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_prov_hdl -= 1;
        free(handle);
    }
    return NETWORK_SUCCESS;
}

int network_do_work(
        network_provider_handle_t handle,
        int32_t workload_factor,
        hg_bulk_t bulk,
        hg_string_t request_structure,
        int32_t* result)
{
    hg_handle_t   h;
    symbio_in_t in;
    symbio_out_t out;
    hg_return_t ret;

    hg_bulk_t local_bulk;
    int32_t * values;
    hg_size_t size;

    in.workload_factor = workload_factor;
    if(bulk != NULL) {
      in.bulk = bulk;
    } else {
      values = (int32_t*)calloc(TRANSFER_SIZE*workload_factor, sizeof(int32_t));
      size = TRANSFER_SIZE*workload_factor*sizeof(int32_t);
      margo_bulk_create(handle->client->mid, 1, (void**)&values, &size, HG_BULK_READ_ONLY, &local_bulk);
      in.bulk = local_bulk;
    }
    in.request_structure = request_structure;

    ret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(ret != HG_SUCCESS)
        return NETWORK_FAILURE;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return NETWORK_FAILURE;
    }

    ret = margo_get_output(h, &out);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return NETWORK_FAILURE;
    }

    *result = out.ret;

    if(bulk == NULL) {
      margo_bulk_free(local_bulk); 
      free(values);
    }
    
    margo_free_output(h, &out);
    margo_destroy(h);
    return NETWORK_SUCCESS;
}
