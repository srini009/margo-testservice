#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include "alpha-server.h"
#include "beta-server.h"
#include "gamma-server.h"

int main(int argc, char** argv)
{
    margo_instance_id mid = margo_init("ofi+verbs", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    hg_addr_t my_address;
    margo_addr_self(mid, &my_address);
    char addr_str[128];
    size_t addr_str_size = 128;
    margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);
    printf("Server running at address %s, with provider id 42\n", addr_str);

    alpha_provider_register(mid, 42, ALPHA_ABT_POOL_DEFAULT, ALPHA_PROVIDER_IGNORE);
    beta_provider_register(mid, 42, BETA_ABT_POOL_DEFAULT, BETA_PROVIDER_IGNORE);
    gamma_provider_register(mid, 42, GAMMA_ABT_POOL_DEFAULT, GAMMA_PROVIDER_IGNORE);

    alpha_create_downstream_handles(mid, 42, my_address);
    beta_create_downstream_handles(mid, 42, my_address);

    margo_addr_free(mid,my_address);

    margo_enable_remote_shutdown(mid);

    margo_wait_for_finalize(mid);

    return 0;
}
